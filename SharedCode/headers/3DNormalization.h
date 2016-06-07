#pragma once

#ifndef NORMALIZATION_3D
#define NORMALIZATION_3D

// dlib headers
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

//dlib headers
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


// eos headers
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
#include "Frontalization.h"

using namespace std;

namespace face_ver {

	void normalize3D(
		dlib::array2d<dlib::rgb_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		dlib::array<dlib::array2d<dlib::rgb_pixel>>& faces,
		cv::Mat& model3D,
		cv::Mat& camera,
		bool extractBackground);

	void normalize3D(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		dlib::array<dlib::array2d<dlib::bgr_pixel>>& faces,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmark_mapper,
		bool extractBackground = true);
}

#endif