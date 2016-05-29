#include <iostream>
#include <fstream>

#include "FaceNormalization.h"
#include "FaceNormalizationService.h"


int main()
{
	std::ofstream input;
	input.open("in.txt", std::ios::out);
	input.close();

	const char* dlibModelPath = "../models/landmarks_points.dat";
	const char* model3DPath   = "../models/3Dmodel.yml";

	face_ver::FaceNormalizationSingleton::initInstance(dlibModelPath, model3DPath);
	face_ver::FaceNormalization faceNorm = face_ver::FaceNormalizationSingleton::getInstance();
	
	const char* inPath = "../samples/sample_1.jpeg";
	const char* outPath = "./results";
	faceNorm.normalizeImage(inPath, outPath, 0);

	return 0;
}