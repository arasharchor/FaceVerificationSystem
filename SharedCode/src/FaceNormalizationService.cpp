#include "FaceNormalizationService.h"
#include "Util.h"

namespace face_ver {
	// static member definition
	FaceNormalization FaceNormalizationSingleton::faceNorm;

	int init(char* landmarksModelPath, char* model3DPath, char* morphableModelPath, char* mappingsFilePath)
	{
		try {

			FaceNormalizationSingleton::initInstance(landmarksModelPath, model3DPath, morphableModelPath, mappingsFilePath);

			return 0;
		}
		catch (...)
		{
			return -1;
		}
	}

	char* normalizeImage(char* path, char* outputPath, int m)
	{
		FaceNormalization faceNorm = FaceNormalizationSingleton::getInstance();
		
		NORM_MODE mode;
		switch (m) {
		case 0:
			mode = NORM_MODE::NORM_2D;
			break;
		case 1:
			mode = NORM_MODE::NORM_3D;
			break;
		case 2:
			mode = NORM_MODE::FRONT;
			break;
		default:
			mode = NORM_MODE::NORM_2D;
		}

		std::vector<std::string> normalizedFaces = faceNorm.normalizeImage(path, outputPath, mode);
		
		string acc;
		if (normalizedFaces.size() > 0)
			acc = normalizedFaces[0];

		for (int i = 1; i < normalizedFaces.size(); i++)
			acc += ";" + normalizedFaces[i];
		
		char* result = strdup(acc.c_str());
		return result;
	}

	char* normalizeImageSet(char* setPath, char* outputPath, int m)
	{
		FaceNormalization faceNorm = FaceNormalizationSingleton::getInstance();
		
		NORM_MODE mode;
		switch (m) {
		case 0:
			mode = NORM_MODE::NORM_2D;
			break;
		case 1:
			mode = NORM_MODE::NORM_3D;
			break;
		case 2:
			mode = NORM_MODE::FRONT;
			break;
		default:
			mode = NORM_MODE::NORM_2D;
		}

		std::vector<std::string> normalizedFaces = faceNorm.normalizeImage(setPath, outputPath, mode);
		string acc;

		if (normalizedFaces.size() > 0)
			acc = normalizedFaces[0];

		for (int i = 1; i < normalizedFaces.size(); i++)
			acc += ";" + normalizedFaces[i];

		char* result = strdup(acc.c_str());
		return result;
	}
}
