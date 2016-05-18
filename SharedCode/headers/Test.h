#ifndef TEST
#define TEST

//dlib headers
#include <dlib/image_io.h>
#include <dlib/matrix.h>
#include <dlib/graph_utils.h>

// std headers
#include <string>

#include "Util.h"

using namespace face_norm; // REMOVE THIS !!

namespace face_ver {
	void dummyTest(const std::string& imagesPath, const std::string& outputPath);
	void classDistance(const std::string& inputFile, const std::string& outputPath);
}

#endif