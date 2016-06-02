#include "FaceNormalization.h"

namespace face_ver {
	std::vector<string> FaceNormalization::normalizeImage(const char* path, const char* outputPath, face_norm::NORM_MODE mode)
	{
		std::string imagePath(path);
		std::string imageName = util::getLeaf(path);

		// load image
		dlib::array2d<dlib::rgb_pixel> img;
		dlib::load_image(img, imagePath.c_str());

		// detect landmarks
		std::vector<dlib::full_object_detection> shapes;
		landmarksDetector.detectLandmarks(img, shapes);

		switch (mode) {
		case face_norm::NORM_MODE::NORM_2D:
		{
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
			dlib::array<dlib::array2d<dlib::rgb_pixel>> faces;
			normalize3D(img, shapes, faces, model3D, cameraMat);

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
		case face_norm::NORM_MODE::FRONT:
		{
			dlib::array<dlib::array2d<dlib::rgb_pixel>> faces;
			frontalize(img, shapes, faces);

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

		}
	}

	std::vector<string> FaceNormalization::normalizeImageSet(const char* path, const char* outputPath, int mode2D)
	{
		return {};
	}
}
