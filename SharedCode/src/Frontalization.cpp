#include "Frontalization.h"

#include <eos/core/LandmarkMapper.hpp>
#include <eos/fitting/nonlinear_camera_estimation.hpp>
#include <eos/fitting/linear_shape_fitting.hpp>
#include <eos/render/utils.hpp>
#include <eos/render/texture_extraction.hpp>
#include <eos/fitting/fitting.hpp>
#include <eos/fitting/nonlinear_camera_estimation.hpp>
#include <eos/fitting/contour_correspondence.hpp>
#include <eos/render/render.hpp>
#include <cereal/cereal.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace face_ver {
	cv::Mat removeFaceBackground(
		dlib::array2d<dlib::bgr_pixel>& img,
		dlib::full_object_detection& shape)
	{
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
		cv::Mat original = dlib::toMat<dlib::array2d<dlib::bgr_pixel>>(img).clone();

		cv::Mat image(original.size(), CV_8UC3, cv::Scalar(0, 0, 0));
		original.copyTo(image, mask);

		//cv::imwrite("original.jpg", image);
		return image;
	}

	HeadOrientation getHeadOrientation(
		dlib::array2d<dlib::bgr_pixel>& img,
		eos::core::LandmarkCollection<cv::Vec2f>& landmarks,
		eos::morphablemodel::MorphableModel& morphable_model, 
		eos::core::LandmarkMapper& landmarkMapper)
	{
		cv::Mat image = dlib::toMat<dlib::array2d<dlib::bgr_pixel>>(img);
		return getHeadOrientation(image, landmarks, morphable_model, landmarkMapper);
	}

	HeadOrientation getHeadOrientation(
		cv::Mat& image,
		eos::core::LandmarkCollection<cv::Vec2f>& landmarks,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmarkMapper)
	{
		// These will be the final 2D and 3D points used for the fitting:
		std::vector<cv::Vec4f> model_points;   // the points in the 3D shape model
		std::vector<int>       vertex_indices; // their vertex indices
		std::vector<cv::Vec2f> image_points;   // the corresponding 2D landmark points
		
		double m2x = 0, m2y = 0;
		double m3x = 0, m3y = 0, m3z = 0;
		for (int i = 0; i < landmarks.size(); ++i) {
			auto converted_name = landmarkMapper.convert(landmarks[i].name);
			if (!converted_name) { // no mapping defined for the current landmark
				continue;
			}
			int vertex_idx = std::stoi(converted_name.get());
			cv::Vec4f vertex = morphable_model.get_shape_model().get_mean_at_point(vertex_idx);
			model_points.emplace_back(vertex);
			vertex_indices.emplace_back(vertex_idx);
			image_points.emplace_back(landmarks[i].coordinates);

			m2x += landmarks[i].coordinates[0];
			m2y += landmarks[i].coordinates[1];
			
			m3x += vertex[0];
			m3y += vertex[1];
			m3z += vertex[2];
		}

		m2x = m2x / model_points.size();
		m2y = m2y / model_points.size();
		m3x = m3x / model_points.size();
		m3y = m3y / model_points.size();
		m3z = m3z / model_points.size();

		// Estimate the camera (pose) from the 2D - 3D point correspondences
		eos::fitting::RenderingParameters rendering_params = eos::fitting::estimate_orthographic_camera(image_points, model_points, image.cols, image.rows);
		cv::Mat affine_from_ortho = get_3x4_affine_camera_matrix(rendering_params, image.cols, image.rows);

		// The 3D head pose can be recovered as follows:
		float yaw_angle = glm::degrees(rendering_params.r_y);
		float pitch_angle = glm::degrees(rendering_params.r_x);
		float roll_angle = glm::degrees(rendering_params.r_z);

		std::cout << "yaw   " << yaw_angle << std::endl;
		std::cout << "pitch " << pitch_angle << std::endl;
		std::cout << "roll  " << roll_angle << std::endl << std::endl;

		// Estimate the shape coefficients by fitting the shape to the landmarks:
		std::vector<float> fitted_coeffs = eos::fitting::fit_shape_to_landmarks_linear(morphable_model, affine_from_ortho, image_points, vertex_indices);

		std::vector<dlib::vector<double, 2> > from_points, to_points;
		for (int i = 0; i < model_points.size(); i++) {
			cv::Mat t = affine_from_ortho * cv::Mat(model_points[i], false);
			from_points.push_back(dlib::point(model_points[i][0], model_points[i][1]));
			to_points.push_back(dlib::point(t.at<float>(0, 0), t.at<float>(0, 1)));
		}


		const dlib::point_transform_affine tform = dlib::find_similarity_transform(from_points, to_points);
		dlib::vector<double, 2> p(1, 0);
		p = tform.get_m() * p;

		// we use as width/height the scale value estimated by the similarity transform
		const double scale = length(p) * 500;

		// get new mean
		cv::Mat m = affine_from_ortho * (cv::Mat_<float>(4, 1) << m3x, m3y, m3z, 1);
		return HeadOrientation(rendering_params.r_y, rendering_params.r_z, rendering_params.r_x, affine_from_ortho, fitted_coeffs, scale, m.at<float>(0), m.at<float>(1));
	}


	static cv::Vec4b getPixelValue(unsigned int x, unsigned int y, cv::Mat& warp_mat_org_inv, cv::Mat& image, eos::render::TextureInterpolation mapping_type)
	{
		if (mapping_type == eos::render::TextureInterpolation::Bilinear) {

			// calculate corresponding position of dst_coord pixel center in image (src)
			cv::Vec3f homogenous_dst_coord(x, y, 1.0f);
			cv::Vec2f src_texel = cv::Mat(warp_mat_org_inv * cv::Mat(homogenous_dst_coord));

			// calculate distances to next 4 pixels
			float distance_upper_left = sqrt(pow(src_texel[0] - floor(src_texel[0]), 2) + pow(src_texel[1] - floor(src_texel[1]), 2));
			float distance_upper_right = sqrt(pow(src_texel[0] - floor(src_texel[0]), 2) + pow(src_texel[1] - ceil(src_texel[1]), 2));
			float distance_lower_left = sqrt(pow(src_texel[0] - ceil(src_texel[0]), 2) + pow(src_texel[1] - floor(src_texel[1]), 2));
			float distance_lower_right = sqrt(pow(src_texel[0] - ceil(src_texel[0]), 2) + pow(src_texel[1] - ceil(src_texel[1]), 2));


			// normalise distances
			float sum_distances = distance_lower_left + distance_lower_right + distance_upper_left + distance_upper_right;
			distance_lower_left /= sum_distances;
			distance_lower_right /= sum_distances;
			distance_upper_left /= sum_distances;
			distance_upper_right /= sum_distances;

			// set color depending on distance from next 4 pixels
			cv::Vec4b pixel;
			for (int color = 0; color < 3; ++color) {
				float color_upper_left = image.at<cv::Vec3b>(floor(src_texel[1]), floor(src_texel[0]))[color] * distance_upper_left;
				float color_upper_right = image.at<cv::Vec3b>(floor(src_texel[1]), ceil(src_texel[0]))[color] * distance_upper_right;
				float color_lower_left = image.at<cv::Vec3b>(ceil(src_texel[1]), floor(src_texel[0]))[color] * distance_lower_left;
				float color_lower_right = image.at<cv::Vec3b>(ceil(src_texel[1]), ceil(src_texel[0]))[color] * distance_lower_right;
				pixel[color] =  color_upper_left + color_upper_right + color_lower_left + color_lower_right;
			}
			pixel[3] = 255;

			return pixel;
		}
		else {
			// calculate corresponding position of dst_coord pixel center in image (src)
			const cv::Mat homogenous_dst_coord(cv::Vec3f(x, y, 1.0f));
			const cv::Vec2f src_texel = cv::Mat(warp_mat_org_inv * homogenous_dst_coord);

			const float tx = cvRound(src_texel[0]);
			const float ty = cvRound(src_texel[1]);

			if ((ty < image.rows) && (tx < image.cols) && tx >= 0 && ty >= 0)
			{
				cv::Vec3b texel = image.at<cv::Vec3b>(ty, tx);
				cv::Vec4b pixel = { texel[0], texel[1], texel[2], 255 };
				return pixel;
			}
			else {
				return{ 0, 0, 0, 0 };
			}
		}
	}

	static void fillTriangle(cv::Vec4f v0, cv::Vec4f v1, cv::Vec4f v2, cv::Vec2f d0, cv::Vec2f d1, cv::Vec2f d2, cv::Mat& image, cv::Mat& isomap, eos::render::TextureInterpolation mapping_type)
	{
		using std::min;
		using std::max;

		cv::Point2f src_tri[3] = { { v0[0], v0[1] },{ v1[0], v1[1] },{ v2[0], v2[1] } };
		cv::Point2f dst_tri[3] = { d0, d1, d2 };

		cv::Mat warp_inv = cv::getAffineTransform(dst_tri, src_tri);
		warp_inv.convertTo(warp_inv, CV_32FC1);
		

		// We now loop over all pixels in the triangle and select, depending on the mapping type, the corresponding texel(s) in the source image
		for (int x = min(dst_tri[0].x, min(dst_tri[1].x, dst_tri[2].x)); x < max(dst_tri[0].x, max(dst_tri[1].x, dst_tri[2].x)); ++x) {
			for (int y = min(dst_tri[0].y, min(dst_tri[1].y, dst_tri[2].y)); y < max(dst_tri[0].y, max(dst_tri[1].y, dst_tri[2].y)); ++y) {
				if (eos::render::detail::is_point_in_triangle(cv::Point2f(x, y), dst_tri[0], dst_tri[1], dst_tri[2])) {
					cv::Vec4b pixel = getPixelValue(x, y, warp_inv, image, mapping_type);
					isomap.at<cv::Vec4b>(y, x) = pixel;
				}
			}
		}
	}

	void frontalize(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		std::vector<cv::Mat>& faces,
		bool removeBackground)
	{
		static const std::string model3Dpath = "./models/sfm_shape_3448.bin";
		static const std::string mappingFilePath = "./models/ibug2did.txt";

		eos::morphablemodel::MorphableModel morphable_model;
		eos::core::LandmarkMapper landmark_mapper;

		try {
			morphable_model = eos::morphablemodel::load_model(model3Dpath);
		}
		catch (const std::runtime_error& e) {
			std::cout << "Error loading the Morphable Model: " << e.what() << std::endl;
			return;
		}

		try {
			landmark_mapper = eos::core::LandmarkMapper(mappingFilePath);
		}
		catch (const std::runtime_error& e) {
			std::cout << "Error loading the Landmark mapper file: " << e.what() << std::endl;
			return;
		}

		frontalize(img, shapes, morphable_model, landmark_mapper, faces, removeBackground);
	}

	void frontalize(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		eos::morphablemodel::MorphableModel& morphable_model,
		eos::core::LandmarkMapper& landmark_mapper,
		std::vector<cv::Mat>& faces,
		bool removeBackground)
	{

		using cv::Mat;
		using cv::Vec2f;
		using cv::Vec3f;
		using cv::Vec4f;
		using cv::Vec3b;
		using std::min;
		using std::max;
		using std::floor;
		using std::ceil;
		using std::sqrt;
		using std::pow;

		const eos::render::TextureInterpolation mapping_type = eos::render::TextureInterpolation::Bilinear;

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

			// get head orientation
			HeadOrientation orientation = getHeadOrientation(img, landmarks, morphable_model, landmark_mapper);

			// Obtain the full mesh with the estimated coefficients:
			eos::render::Mesh mesh = morphable_model.draw_sample(orientation.coeffs, std::vector<float>());

			// remove face background
			//cv::Mat image = removeFaceBackground(img, shape);
			cv::Mat image = dlib::toMat < dlib::array2d<dlib::bgr_pixel>>(img);

			// Render the model to get a depth buffer:
			cv::Mat depthbuffer;
			std::tie(std::ignore, depthbuffer) = eos::render::render_affine(mesh, orientation.cameraMatrix, image.cols, image.rows);

			unsigned int isomapSz = orientation.scale;
			cv::Mat		 isomap = Mat::zeros(isomapSz, isomapSz, CV_8UC4);
			cv::Mat      cameraMat = eos::render::detail::calculate_affine_z_direction(orientation.cameraMatrix);

			typedef std::tuple<Vec2f, Vec2f, Vec2f> triangle;
			std::vector<triangle> toFill;

			std::vector<std::future<void>> results;
		
		
			for (const auto& triangle_indices : mesh.tvi) {
				auto extract_triangle = [&mesh, &cameraMat, &triangle_indices, &depthbuffer, &isomap, &image, &mapping_type, &toFill]() {
					// vertices indices
					const int idx0 = triangle_indices[0];
					const int idx1 = triangle_indices[1];
					const int idx2 = triangle_indices[2];

					// mesh vertices
					cv::Vec4f mv0 = mesh.vertices[idx0];
					cv::Vec4f mv1 = mesh.vertices[idx1];
					cv::Vec4f mv2 = mesh.vertices[idx2];

					// texture vertices
					cv::Vec2f tv0 = mesh.texcoords[idx0];
					cv::Vec2f tv1 = mesh.texcoords[idx1];
					cv::Vec2f tv2 = mesh.texcoords[idx2];

					// project mesh coordinates to screen coordinates
					Vec4f v0 = Mat(cameraMat * Mat(mv0));
					Vec4f v1 = Mat(cameraMat * Mat(mv1));
					Vec4f v2 = Mat(cameraMat * Mat(mv2));

					// if triangle is not visible, try with the mirrored triangle
					if (!eos::render::detail::is_triangle_visible(v0, v1, v2, depthbuffer)) {
						
#ifdef TEXTURE_SIMILARITY
						// texture coordinates -- even if the mesh triangle is mirrored, the texture will be the same
						const Vec2f d0 = { isomap.cols * tv0[0], isomap.rows * tv0[1] - 1.0f };
						const Vec2f d1 = { isomap.cols * tv1[0], isomap.rows * tv1[1] - 1.0f };
						const Vec2f d2 = { isomap.cols * tv2[0], isomap.rows * tv2[1] - 1.0f };

						toFill.push_back(std::make_tuple(d0, d1, d2));
						
#else
						mv0[0] = -mv0[0];
						mv1[0] = -mv1[0];
						mv2[0] = -mv2[0];

						v0 = Mat(cameraMat * Mat(mv0));
						v1 = Mat(cameraMat * Mat(mv1));
						v2 = Mat(cameraMat * Mat(mv2));

						// texture coordinates -- even if the mesh triangle is mirrored, the texture will be the same
						const Vec2f d0 = { isomap.cols * tv0[0], isomap.rows * tv0[1] - 1.0f };
						const Vec2f d1 = { isomap.cols * tv1[0], isomap.rows * tv1[1] - 1.0f };
						const Vec2f d2 = { isomap.cols * tv2[0], isomap.rows * tv2[1] - 1.0f };

						fillTriangle(v0, v1, v2, d0, d1, d2, image, isomap, mapping_type);
#endif
					}
					else {
						// texture coordinates -- even if the mesh triangle is mirrored, the texture will be the same
						const Vec2f d0 = { isomap.cols * tv0[0], isomap.rows * tv0[1] - 1.0f };
						const Vec2f d1 = { isomap.cols * tv1[0], isomap.rows * tv1[1] - 1.0f };
						const Vec2f d2 = { isomap.cols * tv2[0], isomap.rows * tv2[1] - 1.0f };

						fillTriangle(v0, v1, v2, d0, d1, d2, image, isomap, mapping_type);

					}
					// divide triangle in two pieces
					/*
					Vec4f v01, v02, v12;

					for (unsigned int i = 0; i < 4; i++) {
						v01[i] = (v0[i] + v1[i]) / 2;
						v02[i] = (v0[i] + v2[i]) / 2;
						v12[i] = (v1[i] + v2[i]) / 2;
					}

					// N-S split
					if (eos::render::detail::is_triangle_visible(v0, v01, v2, depthbuffer)) {
						Vec2f d01 = Vec2f((d0[0] + d1[0]) / 2, (d0[1] + d1[1]) / 2);
						fillTriangle(v0, v01, v2, d0, d01, d2, image, isomap, mapping_type);
					}
					else if (eos::render::detail::is_triangle_visible(v01, v1, v2, depthbuffer)) {
						Vec2f d01 = Vec2f((d0[0] + d1[0]) / 2, (d0[1] + d1[1]) / 2);
						fillTriangle(v01, v1, v2, d01, d1, d2, image, isomap, mapping_type);
					}

					// E-V split
					else if (eos::render::detail::is_triangle_visible(v0, v1, v12, depthbuffer)) {
						Vec2f d12 = Vec2f((d1[0] + d2[0]) / 2, (d1[1] + d2[1]) / 2);
						fillTriangle(v0, v1, v12, d0, d1, d12, image, isomap, mapping_type);
					}
					else if (eos::render::detail::is_triangle_visible(v0, v12, v2, depthbuffer)) {
						Vec2f d12 = Vec2f((d1[0] + d2[0]) / 2, (d1[1] + d2[1]) / 2);
						fillTriangle(v0, v12, v2, d0, d12, d2, image, isomap, mapping_type);
					}

					// V-E split
					else if (eos::render::detail::is_triangle_visible(v0, v1, v02, depthbuffer)) {
						Vec2f d02 = Vec2f((d0[0] + d2[0]) / 2, (d0[1] + d2[1]) / 2);
						fillTriangle(v0, v1, v02, d0, d1, d02, image, isomap, mapping_type);
					}
					else if (eos::render::detail::is_triangle_visible(v02, v1, v2, depthbuffer)) {
						Vec2f d02 = Vec2f((d0[0] + d2[0]) / 2, (d0[1] + d2[1]) / 2);
						fillTriangle(v02, v1, v2, d02, d1, d2, image, isomap, mapping_type);
					}

					// fill all triangle
					else {

					}
					*/
							
				}; // end lambda auto extract_triangle();
				results.emplace_back(std::async(extract_triangle));
			} // end for all mesh.tvi
			  // Collect all the launched tasks:
			for (auto&& r : results) {
				r.get();
			}

#ifdef TEXTURE_SIMILARITY
			for (auto& tr : toFill) {
				cv::Point2f dst_tri[3] = { std::get<0>(tr), std::get<1>(tr), std::get<2>(tr) };
				for (int x = min(dst_tri[0].x, min(dst_tri[1].x, dst_tri[2].x)); x < max(dst_tri[0].x, max(dst_tri[1].x, dst_tri[2].x)); ++x) {
					for (int y = min(dst_tri[0].y, min(dst_tri[1].y, dst_tri[2].y)); y < max(dst_tri[0].y, max(dst_tri[1].y, dst_tri[2].y)); ++y) {
						if (eos::render::detail::is_point_in_triangle(cv::Point2f(x, y), dst_tri[0], dst_tri[1], dst_tri[2])) {
							isomap.at<cv::Vec4b>(y, x) = isomap.at<cv::Vec4b>(y, isomap.cols - x);
						}
					}
				}
			}
#endif

			// Workaround for the black line in the isomap (see GitHub issue #4):
			if (mesh.texcoords.size() <= 3448)
			{
				isomap = eos::render::detail::interpolate_black_line(isomap);
			}

			// push the isomap:
			faces.push_back(isomap);
		}
	}
}