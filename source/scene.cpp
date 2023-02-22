#include "scene.hpp"

#include <functional>
#include <random>

Scene::Scene ( ) {

	std::default_random_engine engine(std::random_device{}());
	std::uniform_real_distribution<float> distribution(-1.f, 1.f);
	auto rand = std::bind(distribution, engine);

	for (size_t i = 0; i < 200; i++) {

		float x = rand(), y = rand();
		auto position = glm::vec3(x, y, 0.f);
		triangle_positions.push_back(position);

	}

}