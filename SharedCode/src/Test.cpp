#include "Test.h"

namespace face_ver {

	// remove this
	template <typename T>
	static void printVector(const std::string& header, const dlib::matrix<T>& vector, std::ostream& out = std::cout)
	{
		out << header;
		out << "[ " << std::endl;

		for (int i = 0; i < vector.nr(); i++) {
			for (int j = 0; j < vector.nc(); j++) {
				out << vector(i, j) << " ";
			}
			out << ";" << std::endl;
		}

		out << "]";
		out << std::endl;
	}

	void dummyTest(const std::string& inputFile, const std::string& outputPath) 
	{
		// read all features vectors
		// read input paths
		std::vector<std::string> imagePaths;
		std::vector<int>         labels;

		read_csv(inputFile, imagePaths, labels, INT_MAX);

		std::vector<dlib::matrix<double>> features(imagePaths.size());
		for (int i = 0; i < imagePaths.size(); i++)
			dlib::deserialize(imagePaths[i].c_str()) >> features[i];
		
		// open output file
		std::ofstream outFile;
		outFile.open(outputPath, std::ios::out);
		if (!outFile.is_open())
			throw std::exception("couldn't open results output file.");
		

		// compute euclidian distance
		dlib::squared_euclidean_distance distanceCalculator;
		
		// brute test -> compute euclidean distance between all images received as input -- n * (n - 1) / 2 pairs
		for (int i = 0; i < features.size(); i++) {
			const std::string& imageName = imagePaths[i].substr(imagePaths[i].find_last_of("\\//") + 1);
			std::cout << i << " comparing " <<  imageName << std::endl;
			
			for (int j = i + 1; j < features.size(); j++) {
				outFile << std::setw(50) << std::left << imagePaths[i].substr(imagePaths[i].find_last_of("\\\/") + 1);
				outFile << std::setw(50) << std::left << imagePaths[j].substr(imagePaths[j].find_last_of("\\\/") + 1);
				
				double distance = distanceCalculator.operator() < dlib::matrix<double> > (features[i], features[j]);
				
				outFile << distance << std::endl;
			}
		}
	}

	struct ClassInfo {
		unsigned long int    count, samples;
		double minD, maxD, avgD;
		std::string label;
		
		dlib::matrix<double> model;
		
		ClassInfo(std::string label) : label(label) , minD(INT_MAX), maxD(INT_MIN), avgD(0.0), count(0), samples(0) {}
	};

	std::ostream& operator<<(std::ostream& os, const ClassInfo& cls)
	{
		os << "Class: " << cls.label << std::endl;
		
		if (cls.samples > 0)
			os << "nr of samples: " << cls.samples << std::endl;
		
		if (cls.samples != 1) {
			os << "avg distance : " << cls.avgD << std::endl;
			os << "min distance : " << cls.minD << std::endl;
			os << "max distance : " << cls.maxD << std::endl;
			os << "abs error    : " << cls.maxD - cls.minD << std::endl;
		}

		os << std::endl;

		return os;
	}
	
	void classDistance(const std::string& inputFile, const std::string& outputPath)
	{
		// read all features vectors
		// read input paths
		std::vector<std::string> imagePaths;
		std::vector<int>         labels;

		const std::string& outFile = outputPath + "class_results.txt";

		std::ofstream out(outFile, std::ios::out);
		if (!out.is_open())
			throw std::exception("couldn't open out file.");

		read_csv(inputFile, imagePaths, labels, INT_MAX);

		std::vector<dlib::matrix<double>> features(imagePaths.size());
		for (int i = 0; i < imagePaths.size(); i++) {
			dlib::deserialize(imagePaths[i].c_str()) >> features[i];
		}

		// compute euclidian distance
		dlib::squared_euclidean_distance distanceCalculator;

		// dummy approach -- what indices correspond to each label
		std::vector < std::vector<int> > mapping(labels[labels.size() - 1] + 1);
		for (int idx = 0; idx < labels.size(); idx++) {
			mapping[labels[idx]].push_back(idx);
		}

		std::vector<ClassInfo> classes;

		// compute in class info

		for (int i = 0; i < mapping.size(); i++) {
			if (mapping[i].size() == 0)
				continue;

			std::vector<int>& indices = mapping[i];

			// extract class name
			std::string className = imagePaths[indices[0]];
			className = className.substr(className.find_last_of("\/\\") + 1);
			className = className.substr(0, className.find_first_of("0123456789") - 1);

			ClassInfo classInfo(className);

			// compute one to one distance for all the elements of the current class
			for (int m = 0; m < indices.size(); m++) {\
				dlib::matrix<double>& feature1 = features[indices[m]];
				
				for (int n = m + 1; n < indices.size(); n++) {
					dlib::matrix<double>& feature2 = features[indices[n]];

					double distance = distanceCalculator.operator() < dlib::matrix<double> > (feature1, feature2);

					classInfo.avgD += distance;
					classInfo.count++;

					classInfo.minD = std::min(distance, classInfo.minD);
					classInfo.maxD = std::max(distance, classInfo.maxD);
				}
			}

			if (classInfo.count > 0)
				classInfo.avgD /= classInfo.count;

			// compute class model
			classInfo.model = features[indices[0]];
			for (int k = 1; k < indices.size(); k++)
				classInfo.model += features[indices[k]];

			classInfo.samples = indices.size();
			classInfo.model  /= classInfo.samples;

			// add current class
			classes.push_back(classInfo);
		}

		// compute average in class info
		ClassInfo inClass("IN CLASS AVERAGE INFO");
		for (int i = 0; i < classes.size(); i++) {
			if (classes[i].avgD == 0)
				continue;

			inClass.avgD += classes[i].avgD;
			inClass.count++;

			inClass.minD = std::min(inClass.minD, classes[i].minD);
			inClass.maxD = std::max(inClass.maxD, classes[i].maxD);
		}

		inClass.avgD /= inClass.count;

		ClassInfo btwn("BETWEEN CLASS AVERAGE INFO");
		// between class version 2
		for (int i = 0; i < features.size(); i++) {
			for (int j = i + 1; j < features.size(); j++) {
				if (labels[i] == labels[j])
					continue;

				double d = distanceCalculator.operator() < dlib::matrix<double> > (features[i], features[j]);
				btwn.minD = std::min(btwn.minD, d);
				btwn.maxD = std::max(btwn.maxD, d);

				btwn.avgD += d;
				btwn.count++;
			}
		}

		btwn.avgD /= btwn.count;

		out << "X = , Y =\n"
			"Antrenarea s- a facut pe X persoane cu >Y sample - uri pentru fiecare persoana.\n"
			"Urmatoarele rezultate s - au obtinut testand cu ~2000 imagini din LFW.\n"
			"O clasa = o persona cu 1 sau mai multe sample - uri asociate.\n"
			"Pentru fiecare clasa se calculeaza distanta euclidana dintre vectorii de caracteristici asociati fiecarui sample\n"
			"+ valoare medie, valoare minima, valoare maxima.\n"
			"Informatii similare intre clase diferite.\n" << std::endl << std::endl;


		// print general results
		out << inClass;
		out << btwn;

		// print each class
		for (int i = 0; i < classes.size(); i++) {
			out << classes[i];
		}
		
		out.close();
	}
}