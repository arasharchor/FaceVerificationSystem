#ifndef FACE_VERIFICATION
#define FACE_VERIFICATION

// dlib headers
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/graph_utils/function_objects.h>

// opencv headers
#include <opencv2\core.hpp>

// std headers
#include <vector>

// local headers
#include "LandmarksDetection.h"
#include "2DNormalization.h"
#include "FeaturesExtractor.h"
#include "Util.h"

//#define SAVE_ON_DISK

namespace face_ver {

	class FaceVerification {
		face_ver::LandmarksDetector landmarksDetector;
		face_ver::HOGExtractor      hogExtractor;
		face_ver::GABORExtractor    gaborExtractor;

		// output path
		std::string outputPath;

		// dca data
		cv::Mat_<double> Ax, Ay;

		// cca data
		cv::Mat_<double> xcca, ycca;

		cv::Mat xEigenvectors, xMean, yEigenvectors, yMean;

		// pre load data
		bool preloadData;

	public:
		FaceVerification() {}
		FaceVerification(const std::string& landmarksModel, std::string outputPath, bool loadData = false);
		
		// 2D normalization -- write results on disk or place into opencv matrix

		// be sure that you call this function only for a normalized image containing only one face !!
		void extractFeatures(cv::Mat image, std::vector<cv::Mat_<double>>& features, util::Logger& logger);

		// be sure that you call this function only for a normalized image containing only one face !!
		void extractFeatures(const std::string& imagePath, std::vector<cv::Mat_<double>>& features, util::Logger& logger);

		// extract all faces from an input image and normalize them
		void normalize(const std::string& imagePath, std::vector<cv::Mat>& normalizedFaces, util::Logger& logger);

		// extract the first detected face from the input image normalize and save it onto disk
		void normalize(const std::string& imagePath, util::Logger& logger);


		// load DCA projection matrices
		void loadDCAData(std::string path);
		void loadCCAData(std::string path);
		void loadData(std::string path);

		// compare two descriptors using euclidean distance
		double compare(cv::Mat_<double>& one, cv::Mat_<double>& other);

		// perform features fusion using DCA
		cv::Mat_<double> getDescriptor(std::vector<cv::Mat_<double>>& features);
		
		void compareImages(std::string& one, std::string& other);
	
	};
}
#endif