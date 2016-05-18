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

		static void initInstance(const char* modelPath)
		{
			faceNorm = FaceNormalization(modelPath);
		}
	};

	extern "C" {
		EXPORT char* normalizeImage(char* path, char* outputPath, int mode);
		EXPORT char* normalizeImageSet(char* setPath, char* outputPath, int mode);
		EXPORT int   init(char* landmarksModelPath);
	}
}

#endif
