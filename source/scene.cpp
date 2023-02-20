#include "scene.hpp"

#include <functional>
#include <random>

Scene::Scene ( ) {

	std::default_random_engine engine(std::random_device{}());
	std::uniform_real_distribution<float> distribution(-1.f, 1.f);
	auto rand = std::bind(distribution, engine);

	for (size_t i = 0; i < 500; i++) {

		float x = rand(), y = rand();
		auto position = glm::vec3(x, y, 0.f);
		triangle_positions.push_back(position);

	}



    // for (float x = -1.0f; x < 1.0f; x += 0.2f)
	// 	for (float y = -1.0f; y < 1.0f; y += 0.2f)
	// 		triangle_positions.push_back(glm::vec3(x, y, 0.0f));

}