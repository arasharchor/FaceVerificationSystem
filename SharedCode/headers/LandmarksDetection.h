#ifndef LANDMARKS_DETECTOR
#define LANDMARKS_DETECTOR

// dlib headers
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>

// std headers
#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "Util.h"

using namespace std;
using namespace face_norm;

namespace face_ver {

	class LandmarksDetector {
	private:
		std::string modelPath;

		// face detector
		dlib::frontal_face_detector faceDetector;

		// shape predictor
		dlib::shape_predictor shapePredictor;

	public:
		LandmarksDetector() {}
		LandmarksDetector(const string& modelPath);
		
		void detectLandmarks(const std::string& filePath, std::vector<dlib::full_object_detection>& shapes);
		
		void detectLandmarks(dlib::array2d<dlib::rgb_pixel>& img, std::vector<dlib::full_object_detection>& shapes);
		void detectLandmarks(dlib::array2d<dlib::bgr_pixel>& img, std::vector<dlib::full_object_detection>& shapes);
	};
	
	void writeLandmarksPts(std::vector<dlib::full_object_detection> shapes, std::string& outputFilename);

}

#endif