#pragma once

#include <glew.h>

//material que se le pone a cada objeto antes de dibujarlo, controla el brillo especular
//sIntensity: que tan fuerte es el brillo
//shine: que tan concentrado es (mas alto = brillo mas chico y definido, tipo metal o plastico)
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
