#pragma once
#include "PointLight.h"

//luz puntual que solo alumbra dentro de un cono (como una linterna)
//edg es la apertura del cono en grados
class SpotLight :
	public PointLight
{
public:
	SpotLight();

	SpotLight(GLfloat red, GLfloat green, GLfloat blue,
		GLfloat aIntensity, GLfloat dIntensity,
		GLfloat xPos, GLfloat yPos, GLfloat zPos,
		GLfloat xDir, GLfloat yDir, GLfloat zDir,
		GLfloat con, GLfloat lin, GLfloat exp,
		GLfloat edg);

	void UseLight(GLuint ambientIntensityLocation, GLuint ambientcolorLocation,
		GLuint diffuseIntensityLocation, GLuint positionLocation, GLuint directionLocation,
		GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation,
		GLuint edgeLocation);

	//para mover la luz dentro del while: SetFlash cambia posicion y direccion, SetPos solo la posicion
	void SetFlash(glm::vec3 pos, glm::vec3 dir);
	void SetPos(glm::vec3 pos);
	~SpotLight();

private:
	glm::vec3 direction;

	GLfloat edge, procEdge;
};
