#include "2DNormalization.h"
#include <dlib/geometry.h>

namespace face_ver {
	void normalize2D(
		dlib::array2d<dlib::rgb_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		dlib::array<dlib::array2d<dlib::rgb_pixel>>& faces,
		bool extractBackground)
	{
		// Average positions of face points 17-67
		static const double mean_face_shape_x[] = {
			0.000213256, 0.0752622, 0.18113, 0.29077, 0.393397, 0.586856, 0.689483, 0.799124,
			0.904991, 0.98004, 0.490127, 0.490127, 0.490127, 0.490127, 0.36688, 0.426036,
			0.490127, 0.554217, 0.613373, 0.121737, 0.187122, 0.265825, 0.334606, 0.260918,
			0.182743, 0.645647, 0.714428, 0.793132, 0.858516, 0.79751, 0.719335, 0.254149,
			0.340985, 0.428858, 0.490127, 0.551395, 0.639268, 0.726104, 0.642159, 0.556721,
			0.490127, 0.423532, 0.338094, 0.290379, 0.428096, 0.490127, 0.552157, 0.689874,
			0.553364, 0.490127, 0.42689
		};

		static const double mean_face_shape_y[] = {
			0.106454, 0.038915, 0.0187482, 0.0344891, 0.0773906, 0.0773906, 0.0344891,
			0.0187482, 0.038915, 0.106454, 0.203352, 0.307009, 0.409805, 0.515625, 0.587326,
			0.609345, 0.628106, 0.609345, 0.587326, 0.216423, 0.178758, 0.179852, 0.231733,
			0.245099, 0.244077, 0.231733, 0.179852, 0.178758, 0.216423, 0.244077, 0.245099,
			0.780233, 0.745405, 0.727388, 0.742578, 0.727388, 0.745405, 0.780233, 0.864805,
			0.902192, 0.909281, 0.902192, 0.864805, 0.784792, 0.778746, 0.785343, 0.778746,
			0.784792, 0.824182, 0.831803, 0.824182
		};

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
			cv::Mat originalImage = dlib::toMat<dlib::array2d<dlib::rgb_pixel>>(img).clone();
			cv::Mat imageDest(originalImage.size(), CV_8UC3, cv::Scalar(0, 0, 0));
			originalImage.copyTo(imageDest, mask);

			dlib::cv_image<dlib::bgr_pixel> image(imageDest);
			double padding = 0.1;

			std::vector<dlib::vector<double, 2> > from_points, to_points;
			for (unsigned long i = 17; i < shape.num_parts(); ++i)
			{
				// Ignore the lower lip
				if ((55 <= i && i <= 59) || (65 <= i && i <= 67))
					continue;
				// Ignore the eyebrows 
				if (17 <= i && i <= 26)
					continue;

				dlib::vector<double, 2> p;
				p.x() = (padding + mean_face_shape_x[i - 17]) / (2 * padding + 1);
				p.y() = (padding + mean_face_shape_y[i - 17]) / (2 * padding + 1);
				from_points.push_back(p);
				to_points.push_back(shape.part(i));
			}

			const dlib::point_transform_affine tform = dlib::find_similarity_transform(from_points, to_points);
			dlib::vector<double, 2> p(1, 0);
			p = tform.get_m() * p;

			// There are only 3 things happening in a similarity transform.  There is a
			// rescaling, a rotation, and a translation.  

			// rotation parameters.
			double angle = std::atan2(p.y(), p.x());

			// we use as width/height the scale value estimated by the similarity transform
			const double scale = length(p);
			unsigned int width = scale;
			unsigned int height = scale;

			// rectangle centered around the mass center of the face		
			dlib::dpoint m = dlib::dpoint(0, 0);
			for (int i = 0; i < shape.num_parts(); i++)
				m += shape.part(i);
			m = m / shape.num_parts();
			dlib::drectangle faceRect = centered_drect(m, width, height);

			// rotate image with angle. Use mass center as rotation point
			dlib::rectangle rect;
			rect += rotate_point(m, faceRect.tl_corner(), -angle);
			rect += rotate_point(m, faceRect.tr_corner(), -angle);
			rect += rotate_point(m, faceRect.bl_corner(), -angle);
			rect += rotate_point(m, faceRect.br_corner(), -angle);

			dlib::array2d<dlib::rgb_pixel> outImg;
			outImg.set_size(rect.height(), rect.width());

			const dlib::matrix<double, 2, 2> R = dlib::rotation_matrix(angle);
			dlib::point_transform_affine trans = dlib::point_transform_affine(R, -R*dcenter(get_rect(outImg)) + m);
			transform_image(image, outImg, dlib::interpolate_quadratic(), trans);

			faces.push_back(outImg);
		}
	}
}
