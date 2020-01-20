#include "ofxRealsense.h"


//--------------------------------------------------------------
#define __log__(s) {}

/*void __log__(const string &text) {
	ofstream file("log_ofxRealsense.txt", ios::out | ios::app);
	file << ofGetTimestampString() << ": " << text << endl;
	file.close();
}*/

//--------------------------------------------------------------
vector<string> ofxRealsense::get_serials() {	//get list of availble devices serial numbers
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

void ofxRealsense::setup(string serial, const ofxRealsense_Settings &settings) {	    //connect cameras
	serial_ = serial;
	settings_ = settings;
	ofxRealsense_Settings &S = settings_;

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

			device_.connected = false;	//setting it true later only if success
	
			//Start streams
			string stage = "---";
			try {

				stage = "Disable all streams";
				c.disable_all_streams();
				stage = "Enable depth stream";
				if (S.use_depth) {
					c.enable_stream(RS2_STREAM_DEPTH, S.depth_w, S.depth_h, RS2_FORMAT_ANY, S.depth_fps);
				}
				stage = "Enable color stream";
				if (S.use_rgb) {
					c.enable_stream(RS2_STREAM_COLOR, S.rgb_w, S.rgb_h, RS2_FORMAT_ANY, S.rgb_fps);
				}
				stage = "Enable IR stream";
				if (S.use_ir) {
					c.enable_stream(RS2_STREAM_INFRARED, S.depth_w, S.depth_h, RS2_FORMAT_ANY, S.depth_fps);
				}
				//__log__("resolution " + ofToString(w) + " " + ofToString(h) + " " + ofToString(fps));

				//---------------------------------
				stage = "Enable device " + serial;
				c.enable_device(serial);
				// Start the pipeline with the configuration

				stage = "Pipe start";
				device_.profile = device_.pipe.start(c);

				//obtain depth_scale
				if (S.use_depth) {
					stage = "Get selected device";
					rs2::device selected_device = device_.profile.get_device();

					stage = "Get depth sensor";
					auto depth_sensor = selected_device.first<rs2::depth_sensor>();

					//Setting preset
					stage = "Set visual preset";
					if (S.visual_preset > -1) {
						if (depth_sensor.supports(RS2_OPTION_VISUAL_PRESET)) {
							depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, S.visual_preset);

							//High accuracy preset
							//depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_HIGH_ACCURACY);
							//High density preset
							//depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_HIGH_DENSITY);				
						}
					}

					stage = "Obtain depth scale";
					//https://github.com/IntelRealSense/librealsense/issues/2348
					//rs2::device dev = device_.profile.get_device();
					//rs2::depth_sensor ds = dev.query_sensors().front().as<rs2::depth_sensor>();
					device_.depth_scale_mm = depth_sensor.get_depth_scale() * 1000;

					

					stage = "Set using emitter";
					if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
						depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, S.use_emitter);//on/off emitter
					}
				}

				device_.connected = true;	
			}
			catch (std::exception &error) {
				cout << "Exception: Realsense connect error at stage: " << stage <<". Error text: " << error.what() << endl;
				//ofLogError() << error.what();
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
	if (device_.connected) {
		//ofxRealsense_devices_.clear();
		device_.pipe.stop();
		device_.connected = false;
	}
}

//--------------------------------------------------------------
void ofxRealsense::update() {
	frameNew_ = false;

	if (device_.connected) {
		ofxRealsense_Settings &S = settings_;

		std::lock_guard<std::mutex> lock(ofxRealsense_mutex_);
		// Ask each pipeline if there are new frames available
		rs2::frameset frameset;
		if (device_.pipe.poll_for_frames(&frameset)) {
			frameNew_ = true;
			if (S.use_depth) {
				auto depth = frameset.get_depth_frame();
				device_.points = device_.pc.calculate(depth);			//TODO not compute texture coordinates
				device_.depth = depth;
			}
			if (S.use_rgb) {
				device_.color_frame = frameset.get_color_frame();
			}
			if (S.use_ir) {
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
bool ofxRealsense::get_point_cloud(vector<glm::vec3> &pc, int mirrorx, int mirrory, int mirrorz) {				//get point cloud for connected device i
	if (device_.connected && device_.depth.get()) {
		rs2::points &points = device_.points;
		int size = points.size();
		pc.resize(size);
		if (size > 0) {
			float kx = (mirrorx) ? -1000 : 1000;
			float ky = (mirrory) ? -1000 : 1000;
			float kz = (mirrorz) ? -1000 : 1000;
			auto *v = points.get_vertices();
			for (int k = 0; k < size; k++) {
				auto V = v[k];
				pc[k] = glm::vec3(V.x * kx, V.y * ky, V.z * kz);
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
	if (!settings_.use_depth) return false;
	if (device_.connected && device_.depth.get()) {
		auto frame = device_.colorize_frame.process(device_.depth).as<rs2::video_frame>();
		return frame_to_texture(frame, texture);
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_color_texture(ofTexture &texture) {	//get color texture for connected device
	if (!settings_.use_rgb) return false;
	if (device_.connected && device_.color_frame.get()) {
		return frame_to_texture(device_.color_frame, texture);
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_ir_texture(ofTexture &texture) {		//get ir texture for connected device
	if (!settings_.use_ir) return false;
	if (device_.connected && device_.ir_frame.get()) {
		auto frame = device_.colorize_frame.process(device_.ir_frame).as<rs2::video_frame>();
		return frame_to_texture(frame, texture);
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth16_raw(int &w, int &h, uint16_t* &data16) {
	w = 0;
	h = 0;
	data16 = NULL;
	if (!settings_.use_depth) return false;
	if (device_.connected && device_.depth.get()) {
		data16 = (uint16_t*)device_.depth.get_data(); //device_.depth.get_frame_data();
		if (!data16) return false;
		w = settings_.depth_w;		//TODO get straight from depth frame
		h = settings_.depth_h;
		return true;
	}
	return true;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth_pixels_mm(int &w, int &h, vector<float> &data) {
	w = 0;
	h = 0;
	data.clear();

	uint16_t *data16;
	bool result = get_depth16_raw(w, h, data16);
	if (result) {
		//https://github.com/IntelRealSense/librealsense/issues/2348
		data.resize(w*h);
		for (int i = 0; i < w*h; i++) {
			data[i] = data16[i] * device_.depth_scale_mm;
		}
		return true;
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth_pixels_mm(int &w, int &h, vector<unsigned short> &data) {
	w = 0;
	h = 0;
	data.clear();

	uint16_t *data16;
	bool result = get_depth16_raw(w, h, data16);
	if (result) {
		data.resize(w*h);
		for (int i = 0; i < w*h; i++) {
			data[i] = (unsigned short)(data16[i] * device_.depth_scale_mm);	//NOTE: currently we can avoid it, because scale==1
		}
		return true;
	}
	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth_pixels8(float min_dist, float max_dist, int &w, int &h, vector<unsigned char> &data) {
	w = 0;
	h = 0;
	data.clear();

	uint16_t *data16;
	bool result = get_depth16_raw(w, h, data16);
	if (result) {
		data.resize(w*h);
		for (int i = 0; i < w*h; i++) {
			if (data16[i] > 0) {
				data[i] = int(ofMap(data16[i] * device_.depth_scale_mm, min_dist, max_dist, 255, 0, true));
			}
			else {
				data[i] = 0;
			}
		}
		return true;
	}

	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_depth_pixels_rgb(int &w, int &h, vector<unsigned char> &data) {
	if (!settings_.use_depth) return false;
	if (device_.connected && device_.depth.get()) {
		auto frame = device_.colorize_frame.process(device_.depth).as<rs2::video_frame>();
		return frame_to_pixels_rgb(frame, w, h, data);
	}

	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_color_pixels_rgb(int &w, int &h, vector<unsigned char> &data) {
	if (!settings_.use_rgb) return false;
	if (device_.connected && device_.color_frame.get()) {
		return frame_to_pixels_rgb(device_.color_frame, w, h, data);
	}

	return false;
}

//--------------------------------------------------------------
bool ofxRealsense::get_ir_pixels_rgb(int &w, int &h, vector<unsigned char> &data) {
	if (!settings_.use_ir) return false;
	if (device_.connected && device_.ir_frame.get()) {
		auto frame = device_.colorize_frame.process(device_.ir_frame).as<rs2::video_frame>();
		//__log__("calling frame_to_pixels_rgb");
		return frame_to_pixels_rgb(frame, w, h, data);
	}
	return false;
}

//--------------------------------------------------------------
