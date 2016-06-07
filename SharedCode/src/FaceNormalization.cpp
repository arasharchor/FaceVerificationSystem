#include "FaceNormalization.h"

namespace face_ver {
	FaceNormalization::FaceNormalization
	(
		const char* dlibModelPath,
		const char* morphableModelPath,
		const char* lamdmarksMapPath)
		: landmarksDetector(dlibModelPath)
	{
		try {
			morphableModel = eos::morphablemodel::load_model(morphableModelPath);
		}
		catch (const std::runtime_error& e) {
			std::cout << "Error loading the Morphable Model: " << e.what() << std::endl;
		}

		try {
			landmarkMapper = eos::core::LandmarkMapper(lamdmarksMapPath);
		}
		catch (const std::runtime_error& e) {
			std::cout << "Error loading the Landmark mapper file: " << e.what() << std::endl;
		}
	}
	
	std::vector<string> FaceNormalization::normalizeImage(const char* path, const char* outputPath, face_norm::NORM_MODE mode)
	{
		std::string imagePath(path);
		std::string imageName = util::getLeaf(path);

		switch (mode) {
		case face_norm::NORM_MODE::NORM_2D:
		{
			// load image
			dlib::array2d<dlib::rgb_pixel> img;
			dlib::load_image(img, imagePath.c_str());

			// detect landmarks
			std::vector<dlib::full_object_detection> shapes;
			landmarksDetector.detectLandmarks(img, shapes);

			// 2D NORMALIZATION
			dlib::array<dlib::array2d<dlib::rgb_pixel>> faces;
			normalize2D(img, shapes, faces);

			// create output directory
			util::createDirectory(outputPath);

			// write faces one by one
			std::string fullName = imagePath.substr(imagePath.find_last_of("/\\") + 1);
			std::string imageName = fullName.substr(0, fullName.find_last_of("."));
			std::string imageExtension = fullName.substr(fullName.find_last_of(".") + 1);

			std::vector<string> outputPaths;

			for (int i = 0; i < faces.size(); i++) {
				cv::Mat imgRaw = dlib::toMat<dlib::array2d<dlib::rgb_pixel>>(faces[i]);

				std::string faceName = imageName + "_face_" + std::to_string(i) + "." + imageExtension;
				std::string facePath = outputPath;
				util::addPath(facePath, faceName);

				cv::imwrite(facePath, imgRaw);
				outputPaths.push_back(facePath);
			}

			return outputPaths;
		}
		case face_norm::NORM_MODE::NORM_3D:
		{

#ifdef OEPNCV_CALIBRATION
			// load image
			dlib::array2d<dlib::rgb_pixel> img;
			dlib::load_image(img, imagePath.c_str());

			// detect landmarks
			std::vector<dlib::full_object_detection> shapes;
			landmarksDetector.detectLandmarks(img, shapes);

			dlib::array<dlib::array2d<dlib::rgb_pixel>> faces;
			normalize3D(img, shapes, faces, model3D, cameraMat, true);

			// create output directory
			util::createDirectory(outputPath);

			// write faces one by one
			std::string fullName = imagePath.substr(imagePath.find_last_of("/\\") + 1);
			std::string imageName = fullName.substr(0, fullName.find_last_of("."));
			std::string imageExtension = fullName.substr(fullName.find_last_of(".") + 1);

			std::vector<string> outputPaths;

			for (int i = 0; i < faces.size(); i++) {
				cv::Mat imgRaw = dlib::toMat<dlib::array2d<dlib::rgb_pixel>>(faces[i]);

				std::string faceName = imageName + "_face_" + std::to_string(i) + "." + imageExtension;
				std::string facePath = outputPath;
				util::addPath(facePath, faceName);

				cv::imwrite(facePath, imgRaw);
				outputPaths.push_back(facePath);
			}

			return outputPaths;
#else
			// load image
			dlib::array2d<dlib::bgr_pixel> img;
			dlib::load_image(img, imagePath.c_str());

			// detect landmarks
			std::vector<dlib::full_object_detection> shapes;
			landmarksDetector.detectLandmarks(img, shapes);

			// vector used to store extracted faces
			dlib::array<dlib::array2d<dlib::bgr_pixel>> faces;
			normalize3D(img, shapes, faces, morphableModel, landmarkMapper);

			// create output directory
			util::createDirectory(outputPath);

			// write faces one by one
			std::string fullName       = imagePath.substr(imagePath.find_last_of("/\\") + 1);
			std::string imageName      = fullName.substr(0, fullName.find_last_of("."));
			std::string imageExtension = fullName.substr(fullName.find_last_of(".") + 1);

			std::vector<string> outputPaths;
			
			for (int i = 0; i < faces.size(); i++) {
				cv::Mat imgRaw = dlib::toMat<dlib::array2d<dlib::bgr_pixel>>(faces[i]);

				std::string faceName = imageName + "_face_" + std::to_string(i) + "." + imageExtension;
				std::string facePath = outputPath;
				util::addPath(facePath, faceName);

				cv::imwrite(facePath, imgRaw);
				outputPaths.push_back(facePath);
			}

			return outputPaths;

#endif
		}
		case face_norm::NORM_MODE::FRONT:
		{
			// load image
			dlib::array2d<dlib::bgr_pixel> img;
			dlib::load_image(img, imagePath.c_str());

			// detect landmarks
			std::vector<dlib::full_object_detection> shapes;
			landmarksDetector.detectLandmarks(img, shapes);

			std::vector<cv::Mat> faces;
			frontalize(img, shapes, morphableModel, landmarkMapper, faces, true);

			// create output directory
			util::createDirectory(outputPath);

			// write faces one by one
			std::string fullName       = imagePath.substr(imagePath.find_last_of("/\\") + 1);
			std::string imageName      = fullName.substr(0, fullName.find_last_of("."));
			std::string imageExtension = fullName.substr(fullName.find_last_of(".") + 1);

			std::vector<string> outputPaths;

			for (int i = 0; i < faces.size(); i++) {
				cv::Mat imgRaw = faces[i];

				std::string faceName = imageName + "_face_" + std::to_string(i) + "." + imageExtension;
				std::string facePath = outputPath;
				util::addPath(facePath, faceName);

				cv::imwrite(facePath, imgRaw);
				outputPaths.push_back(facePath);
			}

			return outputPaths;
		}
		}
	}

	std::vector<string> FaceNormalization::normalizeImageSet(const char* path, const char* outputPath, int mode2D)
	{
		return {};
	}
}
