#pragma once
#include "Light.h"

//luz que sale de un punto hacia todos lados y se va apagando con la distancia (como un foco)
//la luz se divide entre exp por la distancia al cuadrado, mas lin por la distancia, mas con
//con los valores 1, 0 y 0 no se atenua nada, y entre mas grandes sean lin y exp menos alcance tiene
class PointLight :
	public Light
{
public:
	PointLight();
	PointLight(GLfloat red, GLfloat green, GLfloat blue,
		GLfloat aIntensity, GLfloat dIntensity,
		GLfloat xPos, GLfloat yPos, GLfloat zPos,
		GLfloat con, GLfloat lin, GLfloat exp);

	void UseLight(GLuint ambientIntensityLocation, GLuint ambientcolorLocation,
		GLuint diffuseIntensityLocation, GLuint positionLocation,
		GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation);

	~PointLight();

protected:
	glm::vec3 position;

	GLfloat constant, linear, exponent;
};
