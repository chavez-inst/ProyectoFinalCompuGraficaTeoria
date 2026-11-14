#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	//saca front, right y up a partir de yaw y pitch
	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed * deltaTime;

	//con Shift va tres veces mas rapido
	if (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT])
	{
		velocity = velocity * 3.0f;
	}

	if (keys[GLFW_KEY_W])
	{
		position += front * velocity;
	}

	if (keys[GLFW_KEY_S])
	{
		position -= front * velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		position -= right * velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		position += right * velocity;
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	//para que la camara no se voltee de cabeza
	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	update();
}

//deja la camara dentro de una caja
void Camera::limitarPosicion(glm::vec3 minimo, glm::vec3 maximo)
{
	position = glm::clamp(position, minimo, maximo);
}

//copia posicion y vista, no las velocidades
void Camera::tomarVistaDe(Camera otra)
{
	position = otra.position;
	yaw = otra.yaw;
	pitch = otra.pitch;
	update();
}

//pone la camara en nuevaPosicion viendo hacia objetivo, la usa la camara que sigue al desfile
void Camera::mirarHacia(glm::vec3 nuevaPosicion, glm::vec3 objetivo)
{
	position = nuevaPosicion;
	glm::vec3 direccion = glm::normalize(objetivo - nuevaPosicion);
	yaw = glm::degrees(atan2(direccion.z, direccion.x));
	pitch = glm::degrees(asin(direccion.y));
	update();
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::getCameraPosition()
{
	return position;
}


glm::vec3 Camera::getCameraDirection()
{
	return glm::normalize(front);
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}


Camera::~Camera()
{
}
