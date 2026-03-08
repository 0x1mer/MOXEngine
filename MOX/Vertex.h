#pragma once

#include <glm/glm.hpp>

// Vertex layout: pos(3) + normal(3) + texture coords(2) + color(3) = 11 floats per vertex
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 color;
};

static_assert(sizeof(Vertex) == 44);