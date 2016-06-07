#include <iostream>
#include <fstream>

#include "FaceNormalization.h"
#include "FaceNormalizationService.h"
#include "Util.h"

int main()
{
	std::ofstream input;
	input.open("in.txt", std::ios::out);
	input.close();

	const char* dlibModelPath      = "../models/landmarks_points.dat";
	const char* model3DPath        = "../models/3Dmodel.yml";
	const char* morphableModelPath = "../models/sfm_shape_3448.bin";
	const char* mappingFilePath    = "../models/ibug2did.txt";

	face_ver::FaceNormalizationSingleton::initInstance(dlibModelPath, model3DPath, morphableModelPath, mappingFilePath);
	face_ver::FaceNormalization faceNorm = face_ver::FaceNormalizationSingleton::getInstance();
	
	const char* inPath  = "../samples/sample_1.jpeg";
	const char* outPath = "./results";
	faceNorm.normalizeImage(inPath, outPath, face_norm::NORM_MODE::NORM_3D);

	return 0;
}