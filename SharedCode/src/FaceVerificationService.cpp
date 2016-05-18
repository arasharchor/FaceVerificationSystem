#include "FaceVerificationService.h"

namespace face_ver {
	// static member definition
	FaceVerification FaceVerificationSingleton::faceVer;
	
	int init(const char* landmarksModelPath, const char* ccaModelsPath)
	{
		try {
			
			FaceVerificationSingleton::initInstance(landmarksModelPath, ccaModelsPath);
			
			return 0;
		}
		catch (...)
		{
			return -1;
		}
	}

	double compareImages(const char* onePath, const char* otherPath) {
		static util::Logger logger("tmp.txt");
		const double treshold = 4998.1;

		FaceVerification faceVerif = FaceVerificationSingleton::getInstance();

		if (onePath == nullptr || otherPath == nullptr || strlen(onePath) == 0 || strlen(otherPath) == 0)
			return -3;

		std::cout << "[ FV LIB ] " << onePath << " vs " << otherPath << " ";

		try{
			// normalize images
			std::vector<cv::Mat> normFacesOne;
			faceVerif.normalize(onePath, normFacesOne, logger);

			// normalize images
			std::vector<cv::Mat> normFacesOther;
			faceVerif.normalize(otherPath, normFacesOther, logger);

			if (normFacesOne.size() == 0 || normFacesOther.size() == 0)
				-1;

			// extract features for img one
			std::vector<cv::Mat_<double>> one;
			faceVerif.extractFeatures(normFacesOne[0], one, logger);

			// extract features for img two
			std::vector<cv::Mat_<double>> other;
			faceVerif.extractFeatures(normFacesOther[0], other, logger);

			// get image descriptor for image one
			cv::Mat_<double> descrOne = faceVerif.getDescriptor(one);

			// get image descriptor for image two
			cv::Mat_<double> descrOther = faceVerif.getDescriptor(other);

			// get distance betweeen the two images
			return faceVerif.compare(descrOne, descrOther);
		
		} catch (std::exception e) {
			std::cout << "catched exception " << e.what() << std::endl;
		}

		return -2;
	}
}
