#ifndef FEATURES_EXTRACTOR
#define FEATURES_EXTRACTOR

// dlib headers
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>

// opencv headers
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>

namespace face_ver {

	class HOGExtractor {
		cv::HOGDescriptor hogDescriptor;

	public:
		HOGExtractor();
		cv::Mat_<double> extractFeatures(cv::Mat img);
	};

	class GABORExtractor {
		std::vector<cv::Mat> kernels;
		int orientationsNum, scalesNum;
	
	public:
		GABORExtractor();
		cv::Mat_<double> extractFeatures(cv::Mat img);
	};

}

#endif