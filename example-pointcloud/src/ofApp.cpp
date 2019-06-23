#include "ofApp.h"

#include "ofxRealsense.h"



//--------------------------------------------------------------
void ofApp::setup(){
	vector<string> serials = ofxRealsense::get_serials();
	cout << "Connected RealSense devices:" << endl;
	for (int i = 0; i < serials.size(); i++) {
		cout << i + 1 << ":\t" << serials[i] << endl;
	}
	if (serials.size() == 0) {
		cout << "No devices!" << endl;
	}

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
	int w = 848;
	int h = 480;
	int fps = 30;

	devices_.setup(serials, w, h, fps);

}

//--------------------------------------------------------------
void ofApp::update(){
	devices_.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	if (draw_points) {
		ofMesh mesh;
		cam.begin();

		ofPushMatrix();
		ofScale(0.1, -0.1, 0.1);

		for (int i = 0; i < devices_.size(); i++) {
			vector<glm::vec3> &pc = mesh.getVertices();
			devices_.get_point_cloud(i, pc);

			ofSetColor(255);
			if (i == 0) ofSetColor(255, 0, 0);
			if (i == 1) ofSetColor(0, 255, 0);
			if (i == 2) ofSetColor(0, 0, 255);
			if (i == 3) ofSetColor(255, 255, 0);
			if (i == 4) ofSetColor(255, 0, 255);
			if (i == 5) ofSetColor(0, 255, 255);

			mesh.drawVertices();
		}
		ofPopMatrix();

		cam.end();
	}

	if (draw_textures) {

		ofTexture texture;
		int x = 0;
		for (int i = 0; i < devices_.size(); i++) {
			devices_.get_depth_texture(i, texture);
			if (texture.isAllocated()) {
				ofSetColor(255);
				texture.draw(x, 40, texture.getWidth() / 4, texture.getHeight() / 4);
				x += texture.getWidth() / 4;
			}
		}
	}

	ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 0), 20, 20);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == '1') draw_textures = !draw_textures;
	if (key == '2') draw_points = !draw_points;

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
