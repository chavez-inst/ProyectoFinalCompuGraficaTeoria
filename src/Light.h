#pragma once

#include <glew.h>
#include <glm.hpp>

//clase base de todas las luces: color, intensidad ambiental e intensidad difusa
class Light
{
public:
	Light();
	Light(GLfloat red, GLfloat green, GLfloat blue,
			GLfloat aIntensity, GLfloat dIntensity);

	~Light();

protected:
	glm::vec3 color;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;
};
