#ifndef FACE_VER
#define FACE_VER

#include "FaceVerification.h"

#define EXPORT __declspec(dllexport)

namespace face_ver {
	class FaceVerificationSingleton {
		static FaceVerification faceVer;

	public:
		static FaceVerification& getInstance()
		{
			return faceVer;
		}

		static void initInstance(const char*one, const char*other)
		{
			faceVer = FaceVerification(one, other, true);
		}
	};


	extern "C" {
		EXPORT double compareImages(const char* one, const char* other);
		EXPORT int  init(const char* landmarksModelPath, const char* ccaModelsPath);
	}
}

#endif
