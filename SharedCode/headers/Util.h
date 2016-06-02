#ifndef UTIL_CODE
#define UTIL_CODE

// windows headers
#include <Windows.h>

// std headers
#include <exception>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

namespace util {
	class Logger {
		const std::string fileName;
		std::ofstream out;

	public:
		Logger(std::string fileName);
		~Logger();

		void log(std::string message, const int length = 25);
		void log(double value, const int length = 25);
		void log(int value, const int length = 25);
	};
	
	
	
	void normalizePath(std::string& path);
	std::string getLeaf(const std::string& path);
	std::string getName(const std::string& path);

	void addPath(std::string& basePath, std::string leaf);
	void createDirectory(std::string path);
}


namespace face_norm {
	
	struct Landmark {
		std::string name;
		double x, y;
	};

	typedef std::vector<Landmark> LandmarksSet;

	void read_csv(
		const std::string& filename,
		std::vector<std::string>& imagePaths,
		std::vector<int>& labels,
		unsigned long maxFacesNum,
		char separator = ';');

	void read_csv(
		const std::string& filename,
		std::vector<std::string>& imagePaths,
		unsigned long maxFacesNum,
		char separator = ';');
	
	void read_csv(
		const std::string& filename,
		std::vector<std::pair<std::string, std::string>>& imagePairs,
		std::vector<int>& labels,
		unsigned long maxFacesNum,
		char separator = ';');

	std::vector<LandmarksSet> read_pts_landmarks(std::string filename);
	
	void createDirectory(const std::string& path);

	enum class NORM_MODE {NORM_2D = 0, NORM_3D = 1, FRONT = 2};

}

#endif