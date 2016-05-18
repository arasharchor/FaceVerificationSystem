#ifndef FACE_NORMALIZATION
#define FACE_NORMALIZATION

#include "LandmarksDetection.h"

// dlib headers
#include <dlib/geometry.h>
#include <dlib/opencv.h>

// opencv headers
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// c++ headers
#include <vector>

namespace face_ver {

	class FaceNormalization {
	private:
		LandmarksDetector landmarksDetector;
		
	public:
		FaceNormalization(const char* modelPath) : landmarksDetector(modelPath) {}
		FaceNormalization() {}

		std::vector<string> normalizeImage(const char* path, const char* outputPath, int mode);
		std::vector<string> normalizeImageSet(const char* path, const char* outputPath, int mode);
	};

}

#endif