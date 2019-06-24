#include "ofApp.h"

#include "ofxRealsense.h"



//--------------------------------------------------------------
void ofApp::setup(){
	cout << "---------------------------" << endl;
	cout << "ofxRealsense example-pointcloud example" << endl;
	cout << "It shows point cloud, colored depth images, IR and RGB images" << endl;
	cout << "Use mouse to rotate, scale and move point cloud, double click - reset to default position" << endl;
	cout << "---------------------------" << endl;


	vector<string> serials = ofxRealsense::get_serials();
	cout << "Connected RealSense devices:" << endl;
	for (int i = 0; i < serials.size(); i++) {
		cout << i + 1 << ":\t" << serials[i] << endl;
	}
	if (serials.size() == 0) {
		cout << "No connected RealSense devices!" << endl;
	}
	else {

		/*
		See available resolutions and framerates at ofxRealsense.h comments
		*/
		ofxRealsense_Settings S;
		S.depth_w = 848;
		S.depth_h = 480;
		S.depth_fps = 30;
		S.rgb_w = 1920;
		S.rgb_h = 1080;
		S.rgb_fps = 30;

		device_.setup(serials[0], S);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	device_.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	if (draw_textures) {

		ofTexture texture;
		int x = 0;
		float scale = 0.25;
		if (device_.connected()) {
			for (int pass = 0; pass < 3; pass++) {
				if (pass == 0)	device_.get_depth_texture(texture);
				if (pass == 1)	device_.get_ir_texture(texture);
				if (pass == 2)	device_.get_color_texture(texture);
				if (texture.isAllocated()) {
					ofSetColor(255);
					texture.draw(x, 40, texture.getWidth() * scale, texture.getHeight() * scale);
				}
				x += texture.getWidth() * scale + 20;
			}
		}
	}



	if (draw_points) {
		ofMesh mesh;
		cam.begin();

		ofPushMatrix();
		ofScale(0.1, -0.1, 0.1);

		if (device_.connected()) {
			vector<glm::vec3> &pc = mesh.getVertices();
			device_.get_point_cloud(pc);

			ofSetColor(255, 0, 0);
			mesh.drawVertices();
		}
		ofPopMatrix();
		cam.end();
	}

	ofDrawBitmapStringHighlight("FPS: " + ofToString(ofGetFrameRate(), 0), 20, 20);

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
