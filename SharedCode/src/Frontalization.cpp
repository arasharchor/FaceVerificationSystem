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

			// Extract the texture from the image using given mesh and camera parameters:
			cv::Mat isomap = eos::render::extract_texture(mesh, orientation.cameraMatrix, image);

			// resize to original size
			cv::resize(isomap, isomap, cv::Size(orientation.scale, orientation.scale));

			// push the isomap:
			faces.push_back(isomap);
		}
	}
}