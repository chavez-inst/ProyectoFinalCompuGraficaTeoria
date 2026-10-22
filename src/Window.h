#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

//crea la ventana con GLFW y guarda lo que pasa con el teclado y el mouse
class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat getmuevex() { return muevex; }
	bool getesNoche() { return esNoche; }
	int getcamaraActiva() { return camaraActiva; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }

	~Window();
private:
	GLFWwindow *mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	//valor de ejemplo que cambia con las teclas Y y U
	//para agregar otro se declara la variable aqui, se pone su getter arriba, se le da valor en los constructores y se agrega la tecla en ManejaTeclado
	GLfloat muevex;
	//vale false de tarde y true de noche, cambia con la tecla N
	bool esNoche;
	//camara que se usa, 0 es la libre, 1 la de la fachada de Santo Domingo y 2 la lateral del desfile, cambia con la tecla C
	int camaraActiva;
	bool mouseFirstMoved;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};
