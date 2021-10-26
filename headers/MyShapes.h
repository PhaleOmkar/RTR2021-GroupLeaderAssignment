#pragma once

#include <windows.h>
#include <math.h>

#include <vector>

// Exportable Function Prototypes
extern "C" void drawEllipticalPath(const float glfRadius, float ellipseVertices[6284][3]);
extern "C" void SphereRendering(const float radius, std::vector<float> &sphere_vertices, std::vector<float> &sphere_normals, std::vector<float> &sphere_texCoords, std::vector<unsigned int> &sphere_indices, std::vector<unsigned int> &sphere_line_indices);
