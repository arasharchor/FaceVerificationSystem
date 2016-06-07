#include "3DNormalization.h"

namespace face_ver {

	// uses opencv camera calibration algorithm
	void normalize3D(
		dlib::array2d<dlib::rgb_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		dlib::array<dlib::array2d<dlib::rgb_pixel>>& faces,
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
			for (int i = 0; i < 17; i++)
				ROI_vertices.push_back(cv::Point(shape.part(i).x(), shape.part(i).y()));

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
			cv::Mat originalImage = dlib::toMat<dlib::array2d<dlib::rgb_pixel>>(img).clone();
			cv::Mat imageDest(originalImage.size(), CV_8UC3, cv::Scalar(0, 0, 0));
			originalImage.copyTo(imageDest, mask);
			dlib::cv_image<dlib::bgr_pixel> image(imageDest);

			//vector points
			std::vector<cv::Point2f> points2D;
			std::vector<cv::Point3f> points3D;

			for (int i = 0; i < shape.num_parts(); i++) {
				cv::Point2f point2D = cv::Point2f(shape.part(i).x(), shape.part(i).y());
				points2D.push_back(point2D);
				points3D.push_back(cv::Point3f(model3D.at<float>(i, 0), model3D.at<float>(i, 1), model3D.at<float>(i, 2)));
			}

			// estimate 3D - 2D perspective transformation
			cv::Mat rvec, tvec;
			bool res = cv::solvePnP(model3D, points2D, camera, cv::Mat(), rvec, tvec, false);

			// compute rotation matrix
			cv::Mat R;
			cv::Rodrigues(rvec, R);

			// estimate rotation angles
			cv::Mat mtxR, mtxQ;
			cv::Vec3f angles = cv::RQDecomp3x3(R, mtxR, mtxQ);
			const double roll = angles[2] * dlib::pi / 180;
			std::cout << angles[2] << " " << roll << std::endl;

			// project model on 2D
			cv::Mat projModel;
			projectPoints(model3D, rvec, tvec, camera, cv::Mat(), projModel);
			const int radius = -1000 / tvec.at<double>(2) * 80; // ?! no hard coded values */

			// find 3D model center
			double mx = 0, my = 0, mz = 0;
			for (int i = 0; i < points3D.size(); i++) {
				cv::Point3f p = points3D[i];
				mx += p.x; my += p.y; mz += p.z;
			}

			mx /= points3D.size(); my /= points3D.size(); mz /= points3D.size();

			// project center onto 2D space
			cv::Mat cv_modelC = (cv::Mat_<double>(1, 3) << mx, my, mz), cv_imgC;
			projectPoints(cv_modelC, rvec, tvec, camera, cv::Mat(), cv_imgC);

			// get rectangle with required size
			dlib::dpoint d_imgC(cv_imgC.at<double>(0, 0), cv_imgC.at<double>(0, 1));
			dlib::drectangle faceRect = centered_drect(d_imgC, radius, radius);

			// rotate rectangle to bring roll to zero
			dlib::rectangle rect;
			rect += rotate_point(d_imgC, faceRect.tl_corner(), roll);
			rect += rotate_point(d_imgC, faceRect.tr_corner(), roll);
			rect += rotate_point(d_imgC, faceRect.bl_corner(), roll);
			rect += rotate_point(d_imgC, faceRect.br_corner(), roll);

			dlib::array2d<dlib::rgb_pixel> outImg;
			outImg.set_size(rect.height(), rect.width());

			const dlib::matrix<double, 2, 2> d_R = dlib::rotation_matrix(roll);
			dlib::point_transform_affine trans = dlib::point_transform_affine(d_R, -d_R*dcenter(get_rect(outImg)) + d_imgC);
			transform_image(image, outImg, dlib::interpolate_quadratic(), trans);

			faces.push_back(outImg);
		}
	}

	void normalize3D(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		dlib::array<dlib::array2d<dlib::bgr_pixel>>& faces,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmark_mapper,
		bool extractBackground)
	{
		for (int k = 0; k < shapes.size(); k++) {
			dlib::full_object_detection shape = shapes[k];
			eos::core::LandmarkCollection<cv::Vec2f> landmarks;

			for (int j = 0; j < shape.num_parts(); j++) {
				eos::core::Landmark<cv::Vec2f> landmark;
				landmark.name = std::to_string(j + 1);
				landmark.coordinates[0] = shape.part(j).x();
				landmark.coordinates[1] = shape.part(j).y();
				landmarks.emplace_back(landmark);
			}

			HeadOrientation orientation = getHeadOrientation(img, landmarks, morphable_model, landmark_mapper);
			
			// Obtain the full mesh with the estimated coefficients:
			eos::render::Mesh mesh = morphable_model.draw_sample(orientation.coeffs, std::vector<float>());

			// remove face background
			cv::Mat image = removeFaceBackground(img, shape);
			
			dlib::dpoint m(orientation.mx, orientation.my);
			dlib::drectangle faceRect = centered_drect(m, orientation.scale, orientation.scale);

			// rotate image with angle. Use mass center as rotation point
			dlib::rectangle rect;
			rect += rotate_point(m, faceRect.tl_corner(), -orientation.roll);
			rect += rotate_point(m, faceRect.tr_corner(), -orientation.roll);
			rect += rotate_point(m, faceRect.bl_corner(), -orientation.roll);
			rect += rotate_point(m, faceRect.br_corner(), -orientation.roll);

			dlib::array2d<dlib::bgr_pixel> outImg;
			outImg.set_size(rect.height(), rect.width());

			const dlib::matrix<double, 2, 2> R = dlib::rotation_matrix(-orientation.roll);
			dlib::point_transform_affine trans = dlib::point_transform_affine(R, -R*dcenter(get_rect(outImg)) + m);
			transform_image(img, outImg, dlib::interpolate_quadratic(), trans);

			faces.push_back(outImg);
		}
	}
}