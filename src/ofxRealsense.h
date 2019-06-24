#pragma once
/*
# ofxRealsense - openFrameworks addon for working with Intel Realsense 415,435,435i cameras.
Supported OS: Windows.
Made in oF 0.10.1


Intel Realsense SDK link: https://github.com/IntelRealSense/librealsense

*/


#include "ofMain.h"
#include <mutex>                    // std::mutex, std::lock_guard

#include <librealsense2/rs.hpp>     // Include RealSense Cross Platform API




/*
	Resolutions and framerates:
	Depth/IR:
	424x240
	480x270
	640x360
	640x480
	848x480
	1280x720
	1280x800
	FPS: 6,15,25,30,60,90

	RGB:
	320x180
	320x240
	424x240
	640x360
	640x480
	848x480
	960x540
	1280x720
	1920x1080
	FPS: 6,15,30,60

*/


//Settings for starting device
struct ofxRealsense_Settings {
	int use_depth = 1;
	int use_ir = 1;
	int use_rgb = 1;
	int use_emitter = 1;

	int depth_w = 640;
	int depth_h = 480;
	int depth_fps = 30;

	int rgb_w = 640;
	int rgb_h = 480;
	int rgb_fps = 30;
};


//Holder for Realsense device structures
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



//High-level openFrameworks wrapper
class ofxRealsense {
public:

	static vector<string> get_serials();	//get list of availble devices serial numbers

	//TODO settings for depth postprocessing and laser power
	void setup(string serial, const ofxRealsense_Settings &settings);	    //connect camera

	void update();
	void close();

	string serial() { return serial_; }
	bool connected() { return device_.connected; }



	//TODO
	//callback for connecting/disconnecting devices, see rs-multicam example in SDK
	//auto reconnect if device was connected again

	//TODO optimization
	//not compute texture coordinates if not required


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
	ofxRealsense_Settings settings_;

	bool frameNew_ = false;

	std::mutex ofxRealsense_mutex_;
	
	rs2::context ofxRealsense_ctx;
	ofxRealsense_Device device_;

	bool frame_to_texture(const rs2::video_frame& frame, ofTexture &texture);
	bool frame_to_pixels_rgb(const rs2::video_frame& frame, int &w, int &h, vector<unsigned char> &data);

};
