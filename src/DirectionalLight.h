#pragma once
#include "Light.h"

//luz que le llega a toda la escena desde la misma direccion (como el sol)
class DirectionalLight :
	public Light
{
public:
	DirectionalLight();
	DirectionalLight(GLfloat red, GLfloat green, GLfloat blue,
					GLfloat aIntensity, GLfloat dIntensity,
					GLfloat xDir, GLfloat yDir, GLfloat zDir);

	void UseLight(GLuint ambientIntensityLocation, GLuint ambientcolorLocation,
		GLuint diffuseIntensityLocation, GLuint directionLocation);

	~DirectionalLight();

private:
	glm::vec3 direction;
};
