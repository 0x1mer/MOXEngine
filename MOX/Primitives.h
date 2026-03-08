#pragma once

#include "MOX_API.h"
#include "Mesh.h"

class MOX_API Primitives
{
public:
	static Mesh CreateCube(float size = 1.0f);
	static Mesh CreateSphere(float radius = 1.0f,
		unsigned int sectorCount = 36,
		unsigned int stackCount = 18);
	static Mesh CreateIcoSphere(float radius = 1.0f,
		unsigned int subdivisions = 2);
};