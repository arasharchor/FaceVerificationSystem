#include "FaceNormalizationService.h"

namespace face_ver {
	// static member definition
	FaceNormalization FaceNormalizationSingleton::faceNorm;

	int init(char* landmarksModelPath, char* model3DPath)
	{
		try {

			FaceNormalizationSingleton::initInstance(landmarksModelPath, model3DPath);

			return 0;
		}
		catch (...)
		{
			return -1;
		}
	}

	char* normalizeImage(char* path, char* outputPath, int mode)
	{
		FaceNormalization faceNorm = FaceNormalizationSingleton::getInstance();
		
		std::vector<std::string> normalizedFaces = faceNorm.normalizeImage(path, outputPath, mode);
		
		string acc;
		if (normalizedFaces.size() > 0)
			acc = normalizedFaces[0];

		for (int i = 1; i < normalizedFaces.size(); i++)
			acc += ";" + normalizedFaces[i];
		
		char* result = strdup(acc.c_str());
		return result;
	}

	char* normalizeImageSet(char* setPath, char* outputPath, int mode)
	{
		FaceNormalization faceNorm = FaceNormalizationSingleton::getInstance();
		
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
