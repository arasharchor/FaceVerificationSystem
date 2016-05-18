#include "FaceVerification.h"

namespace face_ver {
	FaceVerification::FaceVerification(
		const std::string& landmarksModel,
		std::string outputPath, bool preloadData) : landmarksDetector(landmarksModel), outputPath(outputPath), preloadData(preloadData)
	{
		if (preloadData)
			loadCCAData(outputPath);
	}

	// be sure that you call this function only for a normalized image containing only one face !!
	void FaceVerification::extractFeatures(cv::Mat image, std::vector<cv::Mat_<double>>& features, util::Logger& logger)
	{
		// resize to this 
		const int imageWidth = 120;
		const int imageHeight = 120;

		// resize image
		cv::resize(image, image, cv::Size(imageWidth, imageHeight));

		// transform into a grey image
		// cvtColor(image, image, CV_RGB2GRAY);

		auto start = chrono::steady_clock::now();

		// extract HOG features
		cv::Mat_<double> HOGfeatures = hogExtractor.extractFeatures(image);

		// extract GABOR features
		cv::Mat_<double> GABORfeatures = gaborExtractor.extractFeatures(image);

		features.push_back(HOGfeatures);
		features.push_back(GABORfeatures);

		auto end = chrono::steady_clock::now();

		double elapsed = (double)(chrono::duration_cast<chrono::milliseconds>(end - start).count()) / 1000;
		logger.log(elapsed);
	}

	// be sure that you call this function only for a normalized image containing only one face !!
	void FaceVerification::extractFeatures(const std::string& imagePath, std::vector<cv::Mat_<double>>& features, util::Logger& logger)
	{
		// log current image name
		std::string imageName = util::getLeaf(imagePath);
		logger.log(imageName);

		// load image
		cv::Mat image = cv::imread(imagePath.c_str(), CV_LOAD_IMAGE_COLOR);

		if (image.data == nullptr)
			return;

		// transform into a grey image
		cvtColor(image, image, CV_RGB2GRAY);

		extractFeatures(image, features, logger);
	}

	void FaceVerification::normalize(const std::string& imagePath, std::vector<cv::Mat>& normalizedFaces, util::Logger& logger)
	{
		// log current image name
		std::string imageName = util::getLeaf(imagePath);
		logger.log(imageName);

		// load image
		dlib::array2d<dlib::rgb_pixel> img;
		dlib::load_image(img, imagePath.c_str());
		
		// LANDMARKS DETECTION

		auto start = chrono::steady_clock::now();

		// detect landmarks
		std::vector<full_object_detection> shapes;
		landmarksDetector.detectLandmarks(img, shapes);

		auto end = chrono::steady_clock::now();

		double elapsed = (double)(chrono::duration_cast<chrono::milliseconds>(end - start).count()) / 1000;
		logger.log(elapsed);


		// 2D NORMALIZATION
		start = chrono::steady_clock::now();

		// array containing normalized faces
		dlib::array<array2d<rgb_pixel>> faces;

		// normalize image
		normalize2D(img, shapes, faces);

		end     = chrono::steady_clock::now();
		elapsed = (double)(chrono::duration_cast<chrono::milliseconds>(end - start).count()) / 1000;
		logger.log(elapsed);
		logger.log((int)shapes.size());
		logger.log("\n");

		for (int i = 0; i < faces.size(); i++) {
			cv::Mat imgRaw = dlib::toMat<array2d<rgb_pixel>>(faces[i]);
			normalizedFaces.push_back(imgRaw.clone());
		}
	}

	// it will get the first detected face (if any) and save it to disk
	void FaceVerification::normalize(const std::string& imagePath, util::Logger& logger)
	{
		
		std::vector<cv::Mat> normalizedImgs;
		normalize(imagePath, normalizedImgs, logger);

		if (normalizedImgs.size() == 0)
			return;

		std::string name  = util::getLeaf(imagePath);
		std::string label = imagePath.substr(0, imagePath.find_last_of("\\/"));
		label = util::getLeaf(label);

		//std::string label = name.substr(0, name.find_first_of("0123456789") - 1);
		std::string labelDir = outputPath;
		util::normalizePath(labelDir);

		util::addPath(labelDir, label);
		util::normalizePath(labelDir);
		util::createDirectory(labelDir);

		// save image
		cv::imwrite(labelDir + name, normalizedImgs[0]);
	}

	double FaceVerification::compare(cv::Mat_<double>& one, cv::Mat_<double>& other)
	{
		// euclidean distance
		return cv::norm(one, other, cv::NORM_L2);
	}

	cv::Mat_<double> FaceVerification::getDescriptor(std::vector<cv::Mat_<double>>& features)
	{
		// lda features extrcat
		//cv::Mat proj = cv::LDA::subspaceProject(xEigenvectors, xMean, features[1]);
		//cv::Mat proj = cv::LDA::subspaceProject(yEigenvectors, yMean, features[1]);
		//return proj;

		if (features.size() == 0)
			throw std::exception("Not enough features extracted.");
		
		
		// dca features extraction
		// fusion by concatenation
		//cv::Mat_<double> z;
		//cv::vconcat(Ax * features[0].t(), Ay * features[1].t(), z);

		// fusion by summation
		//cv::Mat_<double> z = Ax * features[0].t() + Ay * features[1].t();

		//return z.t();
		
		// cca features extraction
		// dimensionality reduction using pca -- for x, y
		cv::Mat_<double> x = features[0] * xcca;
		cv::Mat_<double> y = features[1] * ycca;

		// fusion by summation
		return  x + y;
		
	}

	void FaceVerification::loadDCAData(std::string path)
	{
		// paths to load projection matrices
		std::string AXPath = path;
		util::addPath(AXPath, "Ax.yml");

		std::string AYPath = path;
		util::addPath(AYPath, "Ay.yml");

		cv::FileStorage AXFile(AXPath, cv::FileStorage::READ);
		cv::FileStorage AYFile(AYPath, cv::FileStorage::READ);

		AXFile["Ax"] >> Ax;
		AYFile["Ay"] >> Ay;

		AXFile.release();
		AYFile.release();
	}

	void FaceVerification::loadCCAData(std::string path)
	{
		// paths to load projection matrices
		std::string xcca = path;
		util::addPath(xcca, "xcca.yml");

		std::string ycca = path;
		util::addPath(ycca, "ycca.yml");

		cv::FileStorage xccaF(xcca, cv::FileStorage::READ);
		cv::FileStorage yccaF(ycca, cv::FileStorage::READ);

		if (xccaF.isOpened() == false || yccaF.isOpened() == false)
			throw std::exception("Couldn't open model input files.");


		xccaF["xcca"] >> this->xcca;
		yccaF["ycca"] >> this->ycca;
		
		xccaF.release();
		yccaF.release();
	}

	void FaceVerification::loadData(std::string outputPath)
	{
		cv::FileStorage fex(outputPath + "\\eigen_x.yml", cv::FileStorage::READ);
		fex["eigenvectors"] >> xEigenvectors;
		fex.release();

		cv::FileStorage fmx(outputPath + "\\mean_y.yml", cv::FileStorage::READ);
		fmx["mean"] >> xMean;
		fmx.release();

		cv::FileStorage fey(outputPath + "\\eigen_y.yml", cv::FileStorage::READ);
		fey["eigenvectors"] >> yEigenvectors;
		fey.release();

		cv::FileStorage fmy(outputPath + "\\mean_y.yml", cv::FileStorage::READ);
		fmy["mean"] >> yMean;
		fmy.release();
	}
}