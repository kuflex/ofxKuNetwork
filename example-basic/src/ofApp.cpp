#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	video.load("fingers.mov");
	video.play();
	
	receiver.setup(12345, 1024, true, 10000000);
	sender.setup("localhost", 12345, 1024);
}

//--------------------------------------------------------------
void ofApp::exit() {
	sender.close();
	receiver.close();
}

//--------------------------------------------------------------
void ofApp::update(){
	video.update();
	if (video.isFrameNew()) {
		sender.clearBuffer();
		sender.putPixels(video.getPixelsRef());
		sender.send();
	}
	if (receiver.isDataNew()) {
		ofPixels pix = receiver.getPixels();
		if (pix.getWidth()>0) {
			video_received.setFromPixels(pix);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(64);
	ofSetColor(255);
	video.draw(0, 0);
	if (video_received.getWidth() > 0) {
		video_received.draw(video.getWidth(), 0);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	


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
