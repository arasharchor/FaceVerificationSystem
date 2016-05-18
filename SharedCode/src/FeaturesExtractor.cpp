#include "FeaturesExtractor.h"

namespace face_ver {
	HOGExtractor::HOGExtractor()
	{
		// cell size 
		const int  cellWidth = 4;
		const int  cellHeight = 4;
		const cv::Size cellSize(cellWidth, cellHeight);

		// block size
		const int  blockWidth = 8;
		const int  blockHeight = 8;
		const cv::Size blockSize(blockWidth, blockHeight);

		// HOG descriptor params
		const cv::Size winSize(120, 120);
		const cv::Size blockStride(4, 4);

		// intialize HOG descriptor detector
		hogDescriptor = cv::HOGDescriptor(
			winSize,
			blockSize,
			blockStride,
			cellSize,
			9,
			0,
			-1,
			0,
			0.2,
			0
			);
	}

	cv::Mat_<double> HOGExtractor::extractFeatures(cv::Mat img)
	{
		std::vector<float> hogDescriptors;
		std::vector<cv::Point> locations;

		// compute HOG
		hogDescriptor.compute(img, hogDescriptors, cv::Size(0, 0), cv::Size(0, 0), locations);

		// not the best way!!
		cv::Mat_<double> x(1, hogDescriptors.size());
		std::copy(hogDescriptors.begin(), hogDescriptors.end(), x.begin());

		return x;
	}
	

	GABORExtractor::GABORExtractor()
	{
		// compute GABOR kernels

		// predefined param values
		const double pi = 3.14159265359;
		const double sigma = 3;
		const double lambda = 36;
		const double gamma = 0.5;

		// scaling info
		const int scalesStart = 7;
		const int scalesStep = 2;
		
		// scales number
		scalesNum = 5;

		// orientations number
		orientationsNum = 8;

		std::vector<int>    scales;
		std::vector<double> orientations;

		for (int i = 0; i < orientationsNum; i++)
			orientations.push_back((double)i / orientationsNum * pi);

		for (int i = 0; i < scalesNum; i++)
			scales.push_back(scalesStart + i * scalesStep);

		for (int i = 0; i < scales.size(); i++) {
			for (int j = 0; j < orientations.size(); j++) {
				int    scale = scales[i];
				double orientation = orientations[j];

				cv::Mat kernel = cv::getGaborKernel(cv::Size(scale, scale), sigma, orientation, lambda, gamma);
				kernels.push_back(kernel);
			}
		}
	}

	cv::Mat_<double> GABORExtractor::extractFeatures(cv::Mat img)
	{
		// compute GABOR filters
		std::vector<cv::Mat> gaborFilters;

		for (int i = 0; i < kernels.size(); i++) {
			cv::Mat& kernel = kernels[i];

			cv::Mat filteredImg;
			filter2D(img, filteredImg, CV_32F, kernel);

			gaborFilters.push_back(filteredImg);
		}
		
		// downsample the filters obtained by a factor of 5 
		const float scalingFactor = 1.f / 5;
		const int   scalingWidth  = gaborFilters[0].rows * scalingFactor;
		const int   scalingHeight = gaborFilters[0].cols * scalingFactor;

		for (int i = 0; i < gaborFilters.size(); i++)
			cv::resize(gaborFilters[i], gaborFilters[i], cv::Size(), scalingFactor, scalingFactor);

		const int q = orientationsNum * scalesNum * scalingHeight * scalingWidth;

		cv::Mat_<double> y(1, q, 0.0);

		// probably not the most efficient solution !!
		// copy all images into a single feature vector
		unsigned int index = 0;

		for (int i = 0; i < gaborFilters.size(); i++) {
			gaborFilters[i].reshape(0, 1);

			unsigned int rowWidth = gaborFilters[i].size().width * gaborFilters[i].size().height;

			std::copy(gaborFilters[i].begin<float>(), gaborFilters[i].end<float>(), y.begin() + index);

			index += rowWidth;
		}

		return y;
	}
}