#pragma once

#include "ofMain.h"

#include "ofxRealsense.h"

//
//This example shows how to use ofxRealsense addon for connecting several Intel RealSense cameras
//and show their texture and depth
//See ofxRealsense.h for installing details
//

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofxRealsense devices_;

	ofEasyCam cam;

	bool draw_textures = true;
	bool draw_points = true;

};
