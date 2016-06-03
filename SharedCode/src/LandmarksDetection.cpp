#include "LandmarksDetection.h"

namespace face_ver {
	LandmarksDetector::LandmarksDetector(const std::string& modelPath) : modelPath(modelPath)
	{
		// initialize a frontal face detector --> TO DO: replace with our face detector
		faceDetector = dlib::get_frontal_face_detector();

		// And we also need a shape_predictor.This is the tool that will predict face
		// landmark positions given an image and face bounding box.  Here we are just
		// loading the model from the shape_predictor_68_face_landmarks.dat file you gave
		// as a command line argument.
		dlib::deserialize(modelPath) >> shapePredictor;
	}


	void LandmarksDetector::detectLandmarks
		(
			const std::string& filePath,
			std::vector<dlib::full_object_detection>& shapes
		)
	{
		// load image
		dlib::array2d<dlib::rgb_pixel> img;
		load_image(img, filePath.c_str());

		detectLandmarks(img, shapes);
	}

	void LandmarksDetector::detectLandmarks
		(
			dlib::array2d<dlib::rgb_pixel>& img,
			std::vector<dlib::full_object_detection>& shapes
		)
	{
		// Make the image larger so we can detect small faces.
		pyramid_up(img);

		// Now tell the face detector to give us a list of bounding boxes
		// around all the faces in the image.
		std::vector<dlib::rectangle> dets = faceDetector(img);

		for (unsigned long j = 0; j < dets.size(); ++j)
		{
			dlib::full_object_detection shape = shapePredictor(img, dets[j]);
			shapes.push_back(shape);
		}
	}

	void LandmarksDetector::detectLandmarks
	(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes
	)
	{
		// Make the image larger so we can detect small faces.
		pyramid_up(img);

		// Now tell the face detector to give us a list of bounding boxes
		// around all the faces in the image.
		std::vector<dlib::rectangle> dets = faceDetector(img);

		for (unsigned long j = 0; j < dets.size(); ++j)
		{
			dlib::full_object_detection shape = shapePredictor(img, dets[j]);
			shapes.push_back(shape);
		}
	}
}