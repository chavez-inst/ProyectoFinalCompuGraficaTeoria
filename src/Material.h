#pragma once

#include <glew.h>

//material que se le pone a cada objeto antes de dibujarlo, controla el brillo especular
//sIntensity es que tan fuerte se ve el brillo
//shine es que tan concentrado esta el brillo, entre mas alto se ve mas chico y definido, como en el metal o el plastico
class Material
{
public:
	Material();
	Material(GLfloat sIntensity, GLfloat shine);

	void UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation);

	~Material();

private:
	GLfloat specularIntensity;
	GLfloat shininess;
};
