#include "Frontalization.h"

#include <eos/core/Landmark.hpp>
#include <eos/core/LandmarkMapper.hpp>
#include <eos/fitting/nonlinear_camera_estimation.hpp>
#include <eos/fitting/linear_shape_fitting.hpp>
#include <eos/render/utils.hpp>
#include <eos/render/texture_extraction.hpp>
#include "eos/fitting/fitting.hpp"
#include "eos/fitting/nonlinear_camera_estimation.hpp"
#include "eos/fitting/contour_correspondence.hpp"
#include "eos/render/render.hpp"
#include "eos/render/texture_extraction.hpp"

#include "cereal/cereal.hpp"

#include "glm/gtc/matrix_transform.hpp"


#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace face_ver {

	void frontalize(
		dlib::array2d<dlib::bgr_pixel>& img,
		std::vector<dlib::full_object_detection>& shapes,
		std::vector<cv::Mat>& faces,
		bool removeBackground)
	{

		static const std::string model3Dpath     = "../models/sfm_shape_3448.bin";
		static const std::string mappingFilePath = "../models/ibug2did.txt";
		static const std::string contourJsonFile = "../models/model_contours.json";

		std::string outputfile("results/output");

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

			// Draw the loaded landmarks:
			/*cv::Mat outimg = image.clone();
			for (auto&& lm : landmarks) {
				cv::rectangle(outimg, cv::Point2f(lm.coordinates[0] - 2.0f, lm.coordinates[1] - 2.0f), cv::Point2f(lm.coordinates[0] + 2.0f, lm.coordinates[1] + 2.0f), { 255, 0, 0 });
			}
			cv::imwrite("face_" + std::to_string(k) + "_landmarks.jpg", outimg);
			*/

			eos::morphablemodel::MorphableModel morphable_model;
			try {
				morphable_model = eos::morphablemodel::load_model(model3Dpath);
			}
			catch (const std::runtime_error& e) {
				std::cout << "Error loading the Morphable Model: " << e.what() << std::endl;
				return;
			}
			eos::core::LandmarkMapper landmark_mapper = eos::core::LandmarkMapper(mappingFilePath);

			// These will be the final 2D and 3D points used for the fitting:
			std::vector<cv::Vec4f> model_points;   // the points in the 3D shape model
			std::vector<int>       vertex_indices; // their vertex indices
			std::vector<cv::Vec2f> image_points;   // the corresponding 2D landmark points

			// Sub-select all the landmarks which we have a mapping for (i.e. that are defined in the 3DMM):
			for (int i = 0; i < landmarks.size(); ++i) {
				auto converted_name = landmark_mapper.convert(landmarks[i].name);
				if (!converted_name) { // no mapping defined for the current landmark
					continue;
				}
				int vertex_idx = std::stoi(converted_name.get());
				cv::Vec4f vertex = morphable_model.get_shape_model().get_mean_at_point(vertex_idx);
				model_points.emplace_back(vertex);
				vertex_indices.emplace_back(vertex_idx);
				image_points.emplace_back(landmarks[i].coordinates);
			}

			/*
			eos::fitting::ModelContour model_contour    = eos::fitting::ModelContour::load(contourJsonFile);
			eos::fitting::ContourLandmarks ibug_contour = eos::fitting::ContourLandmarks::load(mappingFilePath);


			// Estimate the camera (pose) from the 2D - 3D point correspondences
			eos::fitting::RenderingParameters rendering_params = eos::fitting::estimate_orthographic_camera(image_points, model_points, image.cols, image.rows);

			// Given the estimated pose, find 2D-3D contour correspondences:
			auto view_model = eos::fitting::get_4x4_modelview_matrix(rendering_params);
			auto ortho_projection = glm::ortho(rendering_params.frustum.l, rendering_params.frustum.r, rendering_params.frustum.b, rendering_params.frustum.t);
			glm::vec4 viewport(0, image.rows, image.cols, -image.rows); // flips y, origin top-left, like in OpenCV	
																		// These are the additional contour correspondences we're going to find and then use:
			std::vector<cv::Vec2f> image_points_contour;
			std::vector<cv::Vec4f> model_points_contour;
			std::vector<int> vertex_indices_contour;
			
			// For each 2D contour landmark, get the corresponding 3D vertex point and vertex id:
			std::tie(image_points_contour, model_points_contour, vertex_indices_contour) = eos::fitting::get_contour_correspondences(landmarks, ibug_contour, model_contour, glm::degrees(rendering_params.r_y), morphable_model, view_model, ortho_projection, viewport);
			
			// Add the contour correspondences to the set of landmarks that we use for the fitting:
			model_points.insert(model_points.end(), model_points_contour.begin(), model_points_contour.end());
			vertex_indices.insert(vertex_indices.end(), vertex_indices_contour.begin(), vertex_indices_contour.end());
			image_points.insert(image_points.end(), image_points_contour.begin(), image_points_contour.end());

			// Re-estimate the pose with all landmarks, including the contour landmarks:
			rendering_params = eos::fitting::estimate_orthographic_camera(image_points, model_points, image.cols, image.rows);
			cv::Mat affine_cam = eos::fitting::get_3x4_affine_camera_matrix(rendering_params, image.cols, image.rows);

			// The 3D head pose can be recovered as follows:
			float yaw_angle = glm::degrees(rendering_params.r_y);
			float pitch_angle = glm::degrees(rendering_params.r_x);
			float roll_angle = glm::degrees(rendering_params.r_z);

			std::cout << "face  " << k + 1 << " / " << shapes.size() << std::endl;
			std::cout << "yaw   " << yaw_angle << std::endl;
			std::cout << "pitch " << pitch_angle << std::endl;
			std::cout << "roll  " << roll_angle << std::endl << std::endl;

			// Estimate the shape coefficients by fitting the shape to the landmarks:
			std::vector<float> fitted_coeffs = eos::fitting::fit_shape_to_landmarks_linear(morphable_model, affine_cam, image_points, vertex_indices);

			// Obtain the full mesh with the estimated coefficients:
			eos::render::Mesh mesh = morphable_model.draw_sample(fitted_coeffs, std::vector<float>());

			// Extract the texture from the image using given mesh and camera parameters:
			cv::Mat isomap = eos::render::extract_texture(mesh, affine_cam, image);

			// push the isomap:
			faces.push_back(isomap);

			*/

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
			cv::imwrite("original.jpg", image);
			

			// Estimate the camera (pose) from the 2D - 3D point correspondences
			eos::fitting::RenderingParameters rendering_params = eos::fitting::estimate_orthographic_camera(image_points, model_points, image.cols, image.rows);
			cv::Mat affine_from_ortho = get_3x4_affine_camera_matrix(rendering_params, image.cols, image.rows);

			// The 3D head pose can be recovered as follows:
			float yaw_angle   = glm::degrees(rendering_params.r_y);
			float pitch_angle = glm::degrees(rendering_params.r_x);
			float roll_angle  = glm::degrees(rendering_params.r_z);

			std::cout << "face  " << k + 1 << " / " << shapes.size() << std::endl;
 			std::cout << "yaw   " << yaw_angle      << std::endl;
			std::cout << "pitch " << pitch_angle    << std::endl;
			std::cout << "roll  " << roll_angle     << std::endl << std::endl;
			
			// Estimate the shape coefficients by fitting the shape to the landmarks:
			std::vector<float> fitted_coeffs = eos::fitting::fit_shape_to_landmarks_linear(morphable_model, affine_from_ortho, image_points, vertex_indices);

			// Obtain the full mesh with the estimated coefficients:
			eos::render::Mesh mesh = morphable_model.draw_sample(fitted_coeffs, std::vector<float>());

			// Extract the texture from the image using given mesh and camera parameters:
			cv::Mat isomap = eos::render::extract_texture(mesh, affine_from_ortho, image);

			// push the isomap:
			faces.push_back(isomap);
		
		}
	}
}