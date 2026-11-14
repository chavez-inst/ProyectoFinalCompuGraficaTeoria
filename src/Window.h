#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

//ventana GLFW, teclado y mouse
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
	bool getdesfilePausado() { return desfilePausado; }
	bool getreiniciarDesfile();
	bool getsinSonido() { return sinSonido; }
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
	//false de tarde, true de noche
	bool esNoche;
	//camara activa, 0 es la libre y de 1 a 4 las fijas
	int camaraActiva;
	//true al soltar una camara fija con F
	bool soltarCamara;
	//la P detiene y continua el desfile, la R lo regresa al atrio
	bool desfilePausado;
	bool reiniciarDesfile;
	//la M apaga y prende todo el audio
	bool sinSonido;
	bool mouseFirstMoved;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};
