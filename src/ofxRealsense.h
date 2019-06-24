#pragma once
/*
# ofxRealsense - openFrameworks addon for working with Intel Realsense 415,435,435i cameras.
Supported OS: Windows.
Made in oF 0.10.1
*/


#include "ofMain.h"
#include <mutex>                    // std::mutex, std::lock_guard

#include <librealsense2/rs.hpp>     // Include RealSense Cross Platform API


struct ofxRealsense_Device
{
	bool connected = false;
	rs2::frame depth;
	rs2::colorizer colorize_frame;
	rs2::pipeline pipe;
	rs2::pipeline_profile profile;

	// Declare pointcloud object, for calculating pointclouds and texture mappings
	rs2::pointcloud pc;
	// We want the points object to be persistent so we can display the last cloud when a frame drops
	rs2::points points;

	rs2::frame color_frame;
	rs2::frame ir_frame;

};


/*
	Resolutions and framerates:
	
	424x240
	480x270
	640x360
	640x480
	848x480
	1280x720
	1280x800

	6,15,25,30,60,90

*/


class ofxRealsense {
public:

	static vector<string> get_serials();	//get list of connected devices' serial numbers

	////TODO color format can differ from depth
	void setup(string serial, int use_depth, int use_color, int use_ir, 
		int w=0, int h=0, int fps=0, int disable_ir_emitter=0);	    //connect cameras


	void update();
	void close();

	string serial() { return serial_; }
	bool connected() { return device_.connected; }

	//TODO
	//add function for returning if camera is actually connected

	//TODO
	//callback for connecting/disconnecting devices, see rs-multicam example in SDK
	//auto reconnect if device was connected again

	//TODO optimization
	//disable RGB stream if not required
	//not compute texture coordinates if not reauired

	//TODO
	//set FPS and resolution

	bool get_point_cloud(vector<glm::vec3> &pc);	//get point cloud for connected device
	bool get_depth_texture(ofTexture &texture);	//get depth texture for connected device
	bool get_color_texture(ofTexture &texture);	//get color texture for connected device
	bool get_ir_texture(ofTexture &texture);		//get ir texture for connected device

	bool get_depth_pixels_rgb(int &w, int &h, vector<unsigned char> &data);
	bool get_color_pixels_rgb(int &w, int &h, vector<unsigned char> &data);
	bool get_ir_pixels_rgb(int &w, int &h, vector<unsigned char> &data);

	bool isFrameNew() { return frameNew_;  }
protected:
	string serial_;

	bool frameNew_ = false;

	std::mutex ofxRealsense_mutex_;
	
	rs2::context ofxRealsense_ctx;
	ofxRealsense_Device device_;

	int use_depth_ = 0;
	int use_color_ = 0;
	int use_ir_ = 0;

	bool frame_to_texture(const rs2::video_frame& frame, ofTexture &texture);
	bool frame_to_pixels_rgb(const rs2::video_frame& frame, int &w, int &h, vector<unsigned char> &data);

};
