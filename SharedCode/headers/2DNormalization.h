#pragma once

#ifndef NORMALIZATION_2D
#define NORMALIZATION_2D

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
#include <opencv2\core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// std headers
#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "Util.h"

using namespace std;

namespace face_ver {

	void normalize2D(
		dlib::array2d<dlib::rgb_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		dlib::array<dlib::array2d<dlib::rgb_pixel>>& faces,
		bool removeBackground = true);
}

#endif