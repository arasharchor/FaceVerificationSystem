#include "Util.h"

namespace util {
	// path manipulation functions

	void normalizePath(std::string& path)
	{
		if (path.length() == 0)
			return;

		if (path[path.length() - 1] != '/' && path[path.length() - 1] != '\\')
			path += "\\";
	}
	
	
	void addPath(std::string& basePath, std::string leaf)
	{
		normalizePath(basePath);

		basePath += leaf;
	}

	std::string getLeaf(const std::string& path) 
	{
		return path.substr(path.find_last_of("\\/") + 1);
	}

	std::string getName(const std::string& path)
	{
		return path.substr(0, path.find_last_of("."));
	}

	void createDirectory(std::string path)
	{

// follows platform dependent code
#ifdef __unix__         

#elif defined(_WIN32) || defined(WIN32) 
		if (CreateDirectory(path.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError()) {}
		else
		{
			// Failed to create directory
			const std::string exceptionStr = "Failed to create directory " + path;
			throw std::exception(exceptionStr.c_str());
		}
#endif
	}


	// logger class
	Logger::Logger(std::string fileName) : fileName(fileName), out(std::ofstream(fileName, std::ios::out))
	{
		if (!out.is_open()) {
			const std::string exceptStr = "Couldn't open logging file " + fileName;
			throw std::exception(exceptStr.c_str());
		}
	}

	Logger::~Logger()
	{
		if (out.is_open())
			out.close();
	}

	void Logger::log(std::string msg, const int length)
	{
		out << msg << std::endl;
	}

	void Logger::log(double msg, const int length)
	{
		out << std::setw(length) << std::left << msg;
	}

	void Logger::log(int msg, const int length)
	{
		out << std::setw(length) << std::left << msg;
	}
}

namespace face_norm {
	void read_csv(
		const std::string& filename,
		std::vector<std::string>& imagePaths,
		std::vector<int>& labels,
		unsigned long maxFacesNum,
		char separator)
	{
		std::ifstream file(filename.c_str(), std::ios::in);
		if (!file.is_open()) {
			throw std::exception("No valid input file was given, please check the given filename.");
		}

		std::string line, path, classlabel;
		while (getline(file, line)) {
			std::stringstream liness(line);
			getline(liness, path, separator);
			getline(liness, classlabel);

			if (!path.empty() && !classlabel.empty()) {
				imagePaths.push_back(path);
				labels.push_back(atoi(classlabel.c_str()));
			}

			if (imagePaths.size() >= maxFacesNum)
				break;
		}

		file.close();
	}

	void read_csv(
		const std::string& filename,
		std::vector<std::string>& imagePaths,
		unsigned long maxFacesNum,
		char separator )
	{
		std::ifstream file(filename.c_str(), std::ios::in);
		if (!file.is_open()) {
			throw std::exception("No valid input file was given, please check the given filename.");
		}

		std::string line, path, classlabel;
		while (getline(file, line)) {
			std::stringstream liness(line);
			getline(liness, path);
		
			if (!path.empty())
				imagePaths.push_back(path);

			if (imagePaths.size() >= maxFacesNum)
				break;
		}

		file.close();
	}

	void read_csv(
		const std::string& filename,
		std::vector<std::pair<std::string, std::string>>& imagePairs,
		std::vector<int>& labels,
		unsigned long maxFacesNum,
		char separator)
	{
		std::ifstream file(filename.c_str(), std::ios::in);
		if (!file) {
			throw std::exception("No valid input file was given, please check the given filename.");
		}

		std::string line, path1, path2, label;
		while (getline(file, line)) {
			std::stringstream liness(line);
			getline(liness, path1, separator);
			getline(liness, path2, separator);
			getline(liness, label);

			if (!path1.empty() && !path2.empty() && !label.empty()) {
				imagePairs.push_back(std::pair<std::string, std::string>(path1, path2));
				labels.push_back(atoi(label.c_str()));
			}

			if (imagePairs.size() >= maxFacesNum)
				break;
		}

		file.close();
	}

	// Read a landmark set (begins with {, ends with }) which contains the landmarks coordinates corresponding to a single face 
	static LandmarksSet read_pts_landmarks(std::ifstream& inputFile)
	{
		LandmarksSet result;
		std::string line;
		
		while (getline(inputFile, line)) {
			if (line == "}")
				break;
			
			std::stringstream lineStream(line);
			Landmark          landmark;

			if (!(lineStream >> landmark.x >> landmark.y)) {
				throw std::exception("Landmark format error while parsing the line: ");
			}

			result.push_back(landmark);
		}
		
		return result;
	}

	/**
	* Reads a landmark file and returns an ordered vector with
	* the 68 2D landmark coordinates.
	*
	* @param[in] filename Path to a .pts file.
	* @return A vector containing sets of landmarks.
	*/
	std::vector<LandmarksSet> read_pts_landmarks(std::string filename)
	{
		// store a landmark set for each detected face
		std::vector<LandmarksSet> landmarks;

		std::ifstream file(filename);
		if (!file.is_open()) {
			throw std::runtime_error(std::string("Could not open landmark file: " + filename));
		}

		// Skip the first 3 lines, they're header lines:
		std::string line;
		getline(file, line); // 'version: 1'
		//getline(file, line); // 'n_points : 68'
		getline(file, line); // '{'

		int ibugId = 1;
		while (getline(file, line)) {
			if (line == "}") { // end of the file
				break;
			}

			if (line != "{") {
				throw std::exception("Any landmark set should start with an '{' on the first line.");
			}

			LandmarksSet currSet = read_pts_landmarks(file);
			if (currSet.size() > 0)
				landmarks.push_back(currSet);
		}

		return landmarks;
	}
	

	void createDirectory(const std::string& path) 
	{
		if (CreateDirectory(path.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			// CopyFile(...)
		}
		else
		{
			// Failed to create directory.
			throw std::exception("failed to create output directory.");
		}
	}

}
