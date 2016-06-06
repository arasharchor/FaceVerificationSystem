#pragma once

#ifndef FACE_NORMALIZATION_SERVICE
#define FACE_NORMALIZATION_SERVICE

#include "FaceNormalization.h"

#define EXPORT __declspec(dllexport)

namespace face_ver {
	class FaceNormalizationSingleton {
		static FaceNormalization faceNorm;

	public:
		static FaceNormalization& getInstance()
		{
			return faceNorm;
		}

		static void initInstance(const char* modelPath, const char* model3DPath)
		{
			faceNorm = FaceNormalization(modelPath);

			// read 3D model
			if (model3DPath != nullptr && strlen(model3DPath) > 0) {
				cv::FileStorage fs;
				fs.open(model3DPath, cv::FileStorage::READ);
				
				cv::Mat_<double> model3D;
				fs["threedee"] >> model3D;
				faceNorm.set3DModel(model3D);

				cv::Mat_<double> cameraMat;
				fs["outA"] >> cameraMat;
				faceNorm.setCameraMat(cameraMat);
			}
		}
	};

	extern "C" {
		EXPORT char* normalizeImage(char* path, char* outputPath, int mode);
		EXPORT char* normalizeImageSet(char* setPath, char* outputPath, int mode);
		EXPORT int   init(char* landmarksModelPath, char* model3DPath);
	}
}

#endif
