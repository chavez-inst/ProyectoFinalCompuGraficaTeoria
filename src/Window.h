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
	bool getesNoche() { return esNoche; }
	int getcamaraActiva() { return camaraActiva; }
	bool getsoltarCamara();
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
	//vale false de tarde y true de noche, cambia con la tecla N
	bool esNoche;
	//camara que se usa, 0 es la libre, 1 la de la fachada de Santo Domingo, 2 la lateral del desfile, 3 la del Zocalo y 4 la aerea
	//cambia con las teclas 1 a 5 o pasando una por una con la C
	int camaraActiva;
	//se vuelve true al presionar F en una camara fija, para que la libre siga desde donde estaba esa camara
	bool soltarCamara;
	bool mouseFirstMoved;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};
