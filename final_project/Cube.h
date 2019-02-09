#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include "Shader.h"
#include <glm/glm.hpp>
class Cube
{
public:
	static bool generatedVBOandVAO;
	static unsigned int vbo;
	static unsigned int vao;
	Shader* shader;



	unsigned int diffuseMapTextureID;
	unsigned int specularMapTextureID;


	//xyz of where this cube is
	glm::vec3 pos;

	Cube();
	~Cube();

	void draw();
};


