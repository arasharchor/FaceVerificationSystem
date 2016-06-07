#pragma once

#ifndef FRONTALIZATION_HEADER
#define FRONTALIZATION_HEADER

// dlib headers
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/geometry.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/opencv.h>

// opencv headers
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// eos header files

#include <eos/core/Landmark.hpp>
#include <eos/core/LandmarkMapper.hpp>
#include <eos/morphablemodel/MorphableModel.hpp>


// std headers
#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "Util.h"


namespace face_ver {

	class HeadOrientation {
	public:

		double yaw, roll, pitch;
		double mx, my, scale;
		unsigned int width, height;
		cv::Mat cameraMatrix;       // 3 x 4 perspective projection matrix
		std::vector<float> coeffs;  // morphable model coefficients  

		HeadOrientation(double y, double r, double p, cv::Mat cameraMat, std::vector<float> coeffs, double scale, double mx, double my) :
			yaw(y), roll(r), pitch(p), cameraMatrix(cameraMat), coeffs(coeffs), scale(scale), mx(mx), my(my)
		{
			// compute scale, width and height from orientation parameters
		}
	};

	void frontalize(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		std::vector<cv::Mat>& faces,
		bool removeBackground = true);
	
	void frontalize(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmark_mapper,
		std::vector<cv::Mat>& faces,
		bool removeBackground = true);

	HeadOrientation getHeadOrientation(
		dlib::array2d<dlib::bgr_pixel>& img,
		eos::core::LandmarkCollection<cv::Vec2f>& landmarks,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmarkMapper);

	HeadOrientation getHeadOrientation(
		cv::Mat& img,
		eos::core::LandmarkCollection<cv::Vec2f>& landmarks,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmarkMapper);

	cv::Mat removeFaceBackground(dlib::array2d<dlib::bgr_pixel>& img, dlib::full_object_detection& shape);

}

#endif