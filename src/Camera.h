#pragma once

#include <glew.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>

#include <glfw3.h>

//camara en primera persona
class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);
	void limitarPosicion(glm::vec3 minimo, glm::vec3 maximo);
	void tomarVistaDe(Camera otra);
	void mirarHacia(glm::vec3 nuevaPosicion, glm::vec3 objetivo);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();
	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;	//en grados
	GLfloat pitch;	//en grados

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	void update();
};
