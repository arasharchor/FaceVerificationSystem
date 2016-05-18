#include <iostream>
#include <FaceNormalizationService.h>

int main()
{
	int ret = face_ver::init("../../models/landmarks_points.dat");
	face_ver::normalizeImage("../../image_1.jpg", ".", 1);

	return 0;
}