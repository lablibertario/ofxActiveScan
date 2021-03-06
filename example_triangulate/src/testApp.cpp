/*
    This file is part of ofxActiveScan.

    ofxActiveScan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ofxActiveScan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ofxActiveScan.  If not, see <http://www.gnu.org/licenses/>.

    Naoto Hieda <micuat@gmail.com> 2013
 */

#include "testApp.h"

using namespace ofxActiveScan;

// entry point
void testApp::setup() {
	if( rootDir.size() > 0 ) {
		init();
		pathLoaded = true;
	} else {
		pathLoaded = false;
	}
}

void testApp::init() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	cameraMode = EASYCAM_MODE;
	
	cv::FileStorage fs(ofToDataPath(rootDir[0] + "/config.yml"), cv::FileStorage::READ);
	fs["proWidth"] >> options.projector_width;
	fs["proHeight"] >> options.projector_height;
	fs["camWidth"] >> camSize.width;
	fs["camHeight"] >> camSize.height;
	fs["vertical_center"] >> options.projector_horizontal_center;
	fs["nsamples"] >> options.nsamples;
	
	cv::FileStorage cfs(ofToDataPath(rootDir[0] + "/calibration.yml"), cv::FileStorage::READ);
	cfs["camIntrinsic"] >> camIntrinsic;
	cfs["camDistortion"] >> camDist;
	cfs["proIntrinsic"] >> proIntrinsic;
	cfs["proDistortion"] >> proDist;
	cfs["proExtrinsic"] >> proExtrinsic;
	
	proSize.width = options.projector_width;
	proSize.height = options.projector_height;
	cout << camIntrinsic << endl;
	cout << proIntrinsic << endl;
	cout << proExtrinsic << endl;
	
	// horizontal and vertical correspondences between projector and camera
	Map2f horizontal(ofToDataPath(rootDir[0] + "/h.map", true));
	Map2f vertical(ofToDataPath(rootDir[0] + "/v.map", true));
	
	ofImage mask, camPerspective;
	ofLoadImage(mask, ofToDataPath(rootDir[0] + "/mask.png"));
	ofLoadImage(camPerspective, ofToDataPath(rootDir[0] + "/camPerspective.jpg"));
	
	Map2f maskMap = toAs(mask);
	Matd camI = toAs(camIntrinsic);
	Matd proI = toAs(proIntrinsic);
	Matd proE = toAs(proExtrinsic);
	
	mesh = triangulate(options, horizontal, vertical, maskMap,
					   camI, camDist,
					   proI, proDist, proE, camPerspective);
	mesh.save(ofToDataPath(rootDir[0] + "/out.ply"));
	
	// set parameters for projection
	proCalibration.setup(proIntrinsic, proSize);
	camCalibration.setup(camIntrinsic, camSize);
	
	ofEnableDepthTest();
}

void testApp::update() {
}

void testApp::draw() {
	if( pathLoaded ) {

		ofBackground(0);
		
		if(cameraMode == EASYCAM_MODE) {
			cam.begin();
			ofScale(1, -1, -1);
			ofScale(1000, 1000, 1000);
			ofTranslate(0, 0, -2);
		} else if(cameraMode == PRO_MODE) {
			ofSetupScreenPerspective(options.projector_width, options.projector_height);
			proCalibration.loadProjectionMatrix(0.0001, 100000000.0);
			cv::Mat m = proExtrinsic;
			cv::Mat extrinsics = (cv::Mat1d(4,4) <<
							  m.at<double>(0,0), m.at<double>(0,1), m.at<double>(0,2), m.at<double>(0,3),
							  m.at<double>(1,0), m.at<double>(1,1), m.at<double>(1,2), m.at<double>(1,3),
							  m.at<double>(2,0), m.at<double>(2,1), m.at<double>(2,2), m.at<double>(2,3),
							  0, 0, 0, 1);
			extrinsics = extrinsics.t();
			glMultMatrixd((GLdouble*) extrinsics.ptr(0, 0));
		} else if(cameraMode == CAM_MODE) {
			ofSetupScreenPerspective(camSize.width, camSize.height);
			camCalibration.loadProjectionMatrix(0.0001, 100000000.0);
		}
		
		mesh.drawVertices();
		
		if(cameraMode == EASYCAM_MODE) {
			cam.end();
		}
		
	}
}

void testApp::keyPressed(int key) {
	if( pathLoaded ) {
			
		switch(key) {
			case '1': cameraMode = EASYCAM_MODE; break;
			case '2': cameraMode = PRO_MODE; break;
			case '3': cameraMode = CAM_MODE; break;
		}
		
	}
	
	if( key == 'f' ) {
		ofToggleFullscreen();
	}
}

void testApp::dragEvent(ofDragInfo dragInfo){
	if( !pathLoaded ) {
		for( int i = 0 ; i < dragInfo.files.size() ; i++ ) {
			rootDir.push_back(dragInfo.files[i]);
		}
		init();
		pathLoaded = true;
	}
}
