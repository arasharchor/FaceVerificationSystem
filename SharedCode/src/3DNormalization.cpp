#include "3DNormalization.h"

#include <opencv2/core/core_c.h>


namespace face_ver {
	static void printMat(char* name, cv::Mat mat)
	{
		std::cout << name << std::endl;
		for (int i = 0; i < mat.size().height; i++) {
			for (int j = 0; j < mat.size().width; j++) {
				std::cout << mat.at<float>(i, j) << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	void normalize3D(
		dlib::array2d<rgb_pixel>& img,
		std::vector<full_object_detection>& shapes,
		dlib::array<array2d<rgb_pixel>>& faces,
		cv::Mat& model3D,
		cv::Mat& camera,
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

			
			/*
			//vector points
			std::vector<CvPoint2D32f> points2D;
			std::vector<CvPoint3D32f> points3D;

			for (int i = 0; i < shape.num_parts(); i++) {
				points2D.push_back(CvPoint2D32f(shape.part(i).x(), -shape.part(i).y()));
				points3D.push_back(CvPoint3D32f(model3D.at<float>(i, 0), model3D.at<float>(i, 1), model3D.at<float>(i, 2)));
			}

			//Estimate the pose 
			//Create the POSIT object with the model points  
			CvPOSITObject* positObject;
			positObject = cvCreatePOSITObject(&points3D[0], (int)points3D.size());


			float* rotation_matrix = new float[9];
			float* translation_vector = new float[3];
			CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 100, 1.0e-4f);
			cvPOSIT(positObject, &points2D[0], 1000, criteria, rotation_matrix, translation_vector);

			//Project the model points with the estimated pose  
			std::vector<CvPoint3D32f> projectedPoints;

			for (size_t p = 0; p<points3D.size(); p++)
			{
				CvPoint3D32f point3D;
				point3D.x = rotation_matrix[0] * points3D[p].x +
					rotation_matrix[1] * points3D[p].y +
					rotation_matrix[2] * points3D[p].z +
					translation_vector[0];
				point3D.y = rotation_matrix[3] * points3D[p].x +
					rotation_matrix[4] * points3D[p].y +
					rotation_matrix[5] * points3D[p].z +
					translation_vector[1];
				point3D.z = rotation_matrix[6] * points3D[p].x +
					rotation_matrix[7] * points3D[p].y +
					rotation_matrix[8] * points3D[p].z +
					translation_vector[2];
				CvPoint2D32f point2D = cvPoint2D32f(0.0, 0.0);
				if (point3D.z != 0)
				{
					point2D.x = 1000 * point3D.x / point3D.z;
					point2D.y = 1000 * point3D.y / point3D.z;
				}
				projectedPoints.push_back(point3D);
			}
			cout << "\n-.- PROJECTED POINTS -.-\n";
			cout << "[\n";
			for (size_t p = 0; p < projectedPoints.size(); p++)
				cout << projectedPoints[p].x << ", " << projectedPoints[p].y << ";\n";
			cout << "]\n";

			cout << "\n-.- ORIGINAL POINTS -.-\n";
			cout << "[\n";
			for (size_t p = 0; p < points2D.size(); p++)
				cout << points2D[p].x << ", " << points2D[p].y << ";\n";
			cout << "]\n";

			cvReleasePOSITObject(&positObject);
			*/

			

			
			//vector points
			std::vector<cv::Point2f> points2D;
			std::vector<cv::Point3f> points3D;
			
			cv::Point2f m(0, 0);

			for (int i = 0; i < shape.num_parts(); i++) {
				cv::Point2f point2D = cv::Point2f(shape.part(i).x(), shape.part(i).y());
				points2D.push_back(point2D);
				m += point2D;
				points3D.push_back(cv::Point3f(model3D.at<float>(i, 0), model3D.at<float>(i, 1), model3D.at<float>(i, 2)));
			}

			m = cv::Point2f(m.x / shape.num_parts(), m.y / shape.num_parts());
			cv::Mat cameraMat = (cv::Mat_<float>(3, 3) <<
				1000, 0, m.x,
				0, 1000, m.y,
				0, 0, 1
				);
			std::cout << cameraMat << std::endl;

			// estimate 3D - 2D transformation
			cv::Mat rvec, tvec;
			bool res = cv::solvePnP(points3D, points2D, cameraMat, cv::Mat(), rvec, tvec);

			// transform rotation matrix
			cv::Mat jacobian, R;
			cv::Rodrigues(rvec, R, jacobian);
			
			cv::Mat mtxR, mtxQ;
			cv::Vec3f angles = cv::RQDecomp3x3(R, mtxR, mtxQ);
			std::cout << R << std::endl;
			std::cout << angles << std::endl;
			//std::cout << tvec << std::endl;
			
			float x = (angles[0] - 90) * pi / 180;
			float y = angles[1] * pi / 180;
			float z = angles[2] * pi / 180;

			std::vector<cv::Point2f> projModel;
			cv::Mat X = (cv::Mat_<float>(3, 3) << 
				1, 0,      0,
				0, cos(x), -sin(x),
				0, sin(x), cos(x)
				);

			cv::Mat Y = (cv::Mat_<float>(3, 3) << 
				cos(y),  0,  sin(y),
				0,       1,  0,
				-sin(y), 0, cos(y)
				);
			
			cv::Mat Z = (cv::Mat_<float>(3, 3) << 
				cos(z), -sin(z), 0,
				sin(z), cos(z),  0, 
				0,      0,       1
				);

			cv::Mat Rot = Z*Y*X;
			std::cout << Rot << std::endl;

			std::vector<cv::Point2f> projPoints;
			float* data = Rot.ptr<float>();

			for (int i = 0; i < points3D.size(); i++) {
				cv::Point3f p = points3D[i];
				float newX = data[0] * p.x + data[1] * p.y + data[2];
				float newY = data[3] * p.x + data[4] * p.y + data[5];
				projPoints.push_back(cv::Point2f(newX, newY));
			}

			std::cout << projPoints << std::endl;

			cv::Mat a = estimateRigidTransform(projPoints, points2D, true);
			
			const double scale = sqrt(a.at<float>(0, 0) * a.at<float>(0, 0) + (a.at<float>(1, 0) * a.at<float>(1, 0)));
			unsigned int width = scale;;
			unsigned int height = scale;
			double angle = z;

			// rectangle centered around the mass center of the face		
			dlib::dpoint me = dlib::dpoint(0, 0);
			for (int i = 0; i < shape.num_parts(); i++)
				me += shape.part(i);
			me = me / shape.num_parts();
			dlib::drectangle faceRect = centered_drect(me, width, height);

			// rotate image with angle. Use mass center as rotation point
			dlib::rectangle rect;
			rect += rotate_point(me, faceRect.tl_corner(), -angle);
			rect += rotate_point(me, faceRect.tr_corner(), -angle);
			rect += rotate_point(me, faceRect.bl_corner(), -angle);
			rect += rotate_point(me, faceRect.br_corner(), -angle);

			dlib::array2d<dlib::rgb_pixel> outImg;
			outImg.set_size(rect.height(), rect.width());

			const dlib::matrix<double, 2, 2> Ro = dlib::rotation_matrix(angle);
			dlib::point_transform_affine trans = dlib::point_transform_affine(Ro, -Ro*dcenter(get_rect(outImg)) + me);
			transform_image(image, outImg, dlib::interpolate_quadratic(), trans);

			faces.push_back(outImg);
			
			//projectPoints(points3D, rvec, tvec, cameraMat, cv::Mat(), projModel);
			//std::cout << projModel << std::endl;

			/*
			// create projection matrix


			std::vector<cv::Point2f> projModel, defaultModel;
			std::vector<cv::Point3f> model;
		    cv::Mat projectedModel;

			for (int i = 0; i < model3D.rows; i++)
				model.push_back(cv::Point3f(model3D.at<float>(i, 0), model3D.at<float>(i, 1), model3D.at<float>(i, 2)));

			cv::Mat zeroRVec = cv::Mat::zeros(1, 3, CV_32F);
			cv::Mat zeroTVec = cv::Mat::zeros(1, 3, CV_32F);
			
			projectPoints(model3D, rvec, tvec, cameraMat, cv::Mat(), projModel);
			projectPoints(model3D, zeroRVec, zeroTVec, cameraMat, cv::Mat(), defaultModel);

			
			// compute euler angles
			cv::Vec3d eulerAngles;
			cv::Mat cameraMat, rotMatrix, transVect, rotMatrixX, rotMatrixY, rotMatrixZ;
			
			decomposeProjectionMatrix(
				projMatrix, 
				cameraMat,
				rotMatrix,
				transVect,
				rotMatrixX,
				rotMatrixY,
				rotMatrixZ,
				eulerAngles);

			
			// project 3D model on points
			
			cv::Mat RR;
			cv::Rodrigues(rvec, RR); // R is 3x3

			RR = RR.t();  // rotation of inverse
			tvec = -RR * tvec; // translation of inverse

			cv::Mat T(4, 4, RR.type()); // T is 4x4
			T(cv::Range(0, 3), cv::Range(0, 3)) = RR * 1; // copies R into T
			T(cv::Range(0, 3), cv::Range(3, 4)) = tvec * 1; // copies tvec into T
			
			std::cout << eulerAngles << std::endl;

			// rotation parameters -- transformed into radians
			double angle = eulerAngles[2] * pi / 180;

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
			*/
		}
	}
}