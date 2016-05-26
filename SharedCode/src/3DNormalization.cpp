#include "3DNormalization.h"

namespace face_ver {

	void normalize3D(
		dlib::array2d<rgb_pixel>& img,
		std::vector<full_object_detection>& shapes,
		dlib::array<array2d<rgb_pixel>>& faces,
		cv::Mat& model3D,
		cv::Mat& cameraMat,
		bool extractBackground)
	{
		if (shapes.size() == 0)
			return;

		for (int i = 0; i < shapes.size(); i++) {
			dlib::full_object_detection shape = shapes[i];
			std::vector<cv::Point> ROI_vertices;

			// points on the face contour
			for (int i = 0; i < 17; i++) {
				ROI_vertices.push_back(cv::Point(shape.part(i).x(), shape.part(i).y()));
			}

			ROI_vertices.push_back(cv::Point(shape.part(26).x(), shape.part(26).y()));
			ROI_vertices.push_back(cv::Point(shape.part(25).x(), shape.part(25).y()));
			ROI_vertices.push_back(cv::Point(shape.part(24).x(), shape.part(24).y()));
			ROI_vertices.push_back(cv::Point(shape.part(19).x(), shape.part(19).y()));

			// Create Polygon from vertices
			std::vector<cv::Point> ROI_Poly;
			cv::approxPolyDP(ROI_vertices, ROI_Poly, 1.0, true);

			// create a mask with the same size as the original image
			cv::Mat mask(cv::Size(img.nc(), img.nr()), CV_8UC3, cv::Scalar(0, 0, 0));
			fillConvexPoly(mask, &ROI_Poly[0], ROI_Poly.size(), cv::Scalar(255, 255, 255), 8, 0);

			// Create new image to store result
			cv::Mat originalImage = dlib::toMat<dlib::array2d<rgb_pixel>>(img).clone();
			cv::Mat imageDest(originalImage.size(), CV_8UC3, cv::Scalar(0, 0, 0));
			originalImage.copyTo(imageDest, mask);
			cv_image<bgr_pixel> image(imageDest);

			// store 2D points into a opencv matrix
			cv::Mat points2D(shape.num_parts(), 2, CV_32F);
			for (int i = 0; i < shape.num_parts(); i++) {
				points2D.at<float>(i, 0) = shape.part(i).x();
				points2D.at<float>(i, 1) = shape.part(i).y();
			}

			// estimate 3D - 2D transformation
			std::cout << model3D.type() << " " << points2D.type() << std::endl;
			cv::Mat rvec, tvec;
			bool res = cv::solvePnP(model3D, points2D, cameraMat, cv::Mat(), rvec, tvec, false);

			// transform rotation matrix
			cv::Mat jacobian, rotCameraMatrix;
			cv::Rodrigues(rvec, rotCameraMatrix, jacobian);

			// compute euler angles
			cv::Vec3d eulerAngles;
			cv::Mat cameraMat, rotMatrix, transVect, rotMatrixX, rotMatrixY, rotMatrixZ;
			double* _r = rotCameraMatrix.ptr<double>();
			double projMatrix[12] = { _r[0],_r[1],_r[2],0,
				_r[3],_r[4],_r[5],0,
				_r[6],_r[7],_r[8],0 };

			decomposeProjectionMatrix(cv::Mat(3, 4, CV_64FC1, projMatrix), 
				cameraMat,
				rotMatrix,
				transVect,
				rotMatrixX,
				rotMatrixY,
				rotMatrixZ,
				eulerAngles);

			// print euler angles
			std::cout << "yaw " << eulerAngles[0] << "\npitch " << eulerAngles[1] << "\nroll " << eulerAngles[2] << std::endl;

			// rotation parameters.
			double angle = eulerAngles[2];

			// rectangle centered around the mass center of the face		
			dlib::dpoint m = dlib::dpoint(0, 0);
			for (int i = 0; i < shape.num_parts(); i++)
				m += shape.part(i);
			m = m / shape.num_parts();
			dlib::drectangle faceRect = centered_drect(m, image.nr(), image.nc());

			// rotate image with angle. Use mass center as rotation point
			rectangle rect;
			rect += rotate_point(m, faceRect.tl_corner(), -angle);
			rect += rotate_point(m, faceRect.tr_corner(), -angle);
			rect += rotate_point(m, faceRect.bl_corner(), -angle);
			rect += rotate_point(m, faceRect.br_corner(), -angle);

			dlib::array2d<rgb_pixel> outImg;
			outImg.set_size(rect.height(), rect.width());

			const matrix<double, 2, 2> R = rotation_matrix(angle);
			point_transform_affine trans = point_transform_affine(R, -R*dcenter(get_rect(outImg)) + m);
			transform_image(image, outImg, interpolate_quadratic(), trans);

			faces.push_back(outImg);
		}
	}
}