#include "ofxRealsense.h"


//--------------------------------------------------------------
#define __log__(s) {}

/*void __log__(const string &text) {
	ofstream file("log_ofxRealsense.txt", ios::out | ios::app);
	file << ofGetTimestampString() << ": " << text << endl;
	file.close();
}*/

//--------------------------------------------------------------
vector<string> ofxRealsense::get_serials() {	//get list of connected devices' serial numbers
	vector<string> list;

	rs2::context ctx;    // Create librealsense context for managing devices

	// Initial population of the device list
	for (auto&& dev : ctx.query_devices()) // Query the list of connected RealSense devices
	{
		list.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
	}

	return list;
}

//--------------------------------------------------------------

void ofxRealsense::setup(string serial, int use_depth, int use_color, int use_ir, int w, int h, int fps, int disable_ir_emitter) {	    //connect cameras
	serial_ = serial;

	use_depth_ = use_depth;
	use_color_ = use_color;
	use_ir_ = use_ir;

	auto &ctx = ofxRealsense_ctx;

	int n = ctx.query_devices().size();
	vector<bool> used(n, false);

	//search
	int k = 0;
	for (auto&& dev : ctx.query_devices()) // Query the list of connected RealSense devices
	{
		if (!used[k] && dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) == serial) {
			cout << "RealSense device starting: " << serial << endl;
			used[k] = true;
			rs2::config c;

			//Start streams
			c.disable_all_streams();
			if (use_depth_) {
				c.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_ANY, fps);
			}
			if (use_color_) {	//TODO color format can differ from depth
				c.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_ANY, fps);
			}
			if (use_ir_) {
				c.enable_stream(RS2_STREAM_INFRARED, w, h, RS2_FORMAT_ANY, fps);
			}
			__log__("resolution " + ofToString(w) + " " + ofToString(h) + " " + ofToString(fps));

			//---------------------------------
			c.enable_device(serial);
			// Start the pipeline with the configuration
			device_.profile = device_.pipe.start(c);
			device_.connected = true;

			//disable emitter
			rs2::device selected_device = device_.profile.get_device();
			auto depth_sensor = selected_device.first<rs2::depth_sensor>();

			if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
				depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1 - disable_ir_emitter);//on/off emitter
			}
			//if (depth_sensor.supports(RS2_OPTION_LASER_POWER)) {
			//	depth_sensor.set_option(RS2_OPTION_LASER_POWER, 0.f); // Disable laser
			//}

			/*
			//Control laser and emitter
			//https://github.com/IntelRealSense/librealsense/wiki/API-How-To
			rs2::pipeline pipe;
			rs2::pipeline_profile selection = pipe.start();
			rs2::device selected_device = selection.get_device();
			auto depth_sensor = selected_device.first<rs2::depth_sensor>();

			if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED))
			{
			depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1.f); // Enable emitter
			depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f); // Disable emitter
			}
			if (depth_sensor.supports(RS2_OPTION_LASER_POWER))
			{
			// Query min and max values:
			auto range = depth_sensor.get_option_range(RS2_OPTION_LASER_POWER);
			depth_sensor.set_option(RS2_OPTION_LASER_POWER, range.max); // Set max power
			depth_sensor.set_option(RS2_OPTION_LASER_POWER, 0.f); // Disable laser
		}
		*/


			break;
		}
		k++;
	}

}

//--------------------------------------------------------------
void ofxRealsense::close() {
	//ofxRealsense_devices_.clear();
	device_.pipe.stop();
	device_.connected = false;
}

//--------------------------------------------------------------
void ofxRealsense::update() {
	frameNew_ = false;

	std::lock_guard<std::mutex> lock(ofxRealsense_mutex_);
	if (device_.connected) {
		// Ask each pipeline if there are new frames available
		rs2::frameset frameset;
		if (device_.pipe.poll_for_frames(&frameset)) {
			frameNew_ = true;
			if (use_depth_) {
				auto depth = frameset.get_depth_frame();
				device_.points = device_.pc.calculate(depth);			//TODO not compute texture coordinates
				device_.depth = depth;
			}
			if (use_color_) {
				device_.color_frame = frameset.get_color_frame();
			}
			if (use_ir_) {
				device_.ir_frame = frameset.get_infrared_frame(0);	//TODO choose 0,1
			}

			//cout << device.points.size() << endl;

			/*cout << "get frames " << frameset.size() << endl;
			for (int i = 0; i < frameset.size(); i++)
			{
				rs2::frame new_frame = frameset[i];
				int stream_id = new_frame.get_profile().unique_id();
				int stream_type = new_frame.get_profile().stream_type();
				cout << "\t" << stream_type << endl;

				auto depth = frames.get_depth_frame();


				//view.second.frames_per_stream[stream_id] = view.second.colorize_frame.process(new_frame); //update view port with the new stream
			}*/

		}
	}
}

//--------------------------------------------------------------
bool ofxRealsense::get_point_cloud(vector<glm::vec3> &pc) {				//get point cloud for connected device i
	if (device_.connected && device_.depth.get()) {
		rs2::points &points = device_.points;
		int size = points.size();
		pc.resize(size);
		if (size > 0) {
			auto *v = points.get_vertices();
			for (int k = 0; k < size; k++) {
				auto V = v[k];
				pc[k] = glm::vec3(V.x * 1000, -V.y * 1000, V.z * 1000);
			}
		}
		return true;
	}
	return false;
}


//--------------------------------------------------------------
bool ofxRealsense::frame_to_texture(const rs2::video_frame& frame, ofTexture &texture) {
	auto format = frame.get_profile().format();
	int w = frame.get_width();
	int h = frame.get_height();
	auto stream = frame.get_profile().stream_type();

	if (format == RS2_FORMAT_Y8) {
		texture.allocate(w, h, GL_LUMINANCE);
		texture.loadData((unsigned char *)frame.get_data(), w, h, GL_LUMINANCE);
		return true;
	}
	if (format == RS2_FORMAT_RGB8) {
		texture.allocate(w, h, GL_RGB);
		texture.loadData((unsigned char *)frame.get_data(), w, h, GL_RGB);
		return true;
	}
	if (format == RS2_FORMAT_RGBA8) {
		texture.allocate(w, h, GL_RGBA);
		texture.loadData((unsigned char *)frame.get_data(), w, h, GL_RGBA);
		return true;
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::frame_to_pixels_rgb(const rs2::video_frame& frame, int &w, int &h, vector<unsigned char> &data) {
	auto format = frame.get_profile().format();
	w = frame.get_width();
	h = frame.get_height();
	auto stream = frame.get_profile().stream_type();

	unsigned char *input = (unsigned char *)frame.get_data();
	data.resize(w*h * 3);

	if (format == RS2_FORMAT_Y8) {
		int k = 0;
		for (int i = 0; i < w*h; i++) {
			data[k++] = input[i];
			data[k++] = input[i];
			data[k++] = input[i];
		}
		//__log__("RS2_FORMAT_Y8 ok");
		return true;
	}
	if (format == RS2_FORMAT_RGB8) {
		memcpy(&data[0], input, w*h * 3);
		//__log__("RS2_FORMAT_RGB8 ok");
		return true;
	}
	if (format == RS2_FORMAT_RGBA8) {
		int k = 0;
		int j = 0;
		for (int i = 0; i < w*h; i++) {
			data[k++] = input[j++];
			data[k++] = input[j++];
			data[k++] = input[j++];
			j++;
		}
		//__log__("RS2_FORMAT_RGBA8 ok");
		return true;
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth_texture(ofTexture &texture) {	//get depth texture for connected device i
	if (!use_depth_) return false;
	if (device_.connected && device_.depth.get()) {
		auto frame = device_.colorize_frame.process(device_.depth).as<rs2::video_frame>();
		return frame_to_texture(frame, texture);
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_color_texture(ofTexture &texture) {	//get color texture for connected device
	if (!use_color_) return false;
	if (device_.connected && device_.color_frame.get()) {
		return frame_to_texture(device_.color_frame, texture);
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_ir_texture(ofTexture &texture) {		//get ir texture for connected device
	if (!use_ir_) return false;
	if (device_.connected && device_.ir_frame.get()) {
		auto frame = device_.colorize_frame.process(device_.ir_frame).as<rs2::video_frame>();
		return frame_to_texture(frame, texture);
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth_pixels_rgb(int &w, int &h, vector<unsigned char> &data) {
	if (!use_depth_) return false;
	if (device_.connected && device_.depth.get()) {
		auto frame = device_.colorize_frame.process(device_.depth).as<rs2::video_frame>();
		return frame_to_pixels_rgb(frame, w, h, data);
	}

	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_color_pixels_rgb(int &w, int &h, vector<unsigned char> &data) {
	if (!use_color_) return false;
	if (device_.connected && device_.color_frame.get()) {
		return frame_to_pixels_rgb(device_.color_frame, w, h, data);
	}

	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_ir_pixels_rgb(int &w, int &h, vector<unsigned char> &data) {
	if (!use_ir_) return false;
	if (device_.connected && device_.ir_frame.get()) {
		auto frame = device_.colorize_frame.process(device_.ir_frame).as<rs2::video_frame>();
		//__log__("calling frame_to_pixels_rgb");
		return frame_to_pixels_rgb(frame, w, h, data);
	}
	return false;
}

//--------------------------------------------------------------
