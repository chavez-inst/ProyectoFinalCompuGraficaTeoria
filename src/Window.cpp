#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	mainWindow = NULL;
	bufferWidth = 0;
	bufferHeight = 0;
	lastX = 0.0f;
	lastY = 0.0f;
	xChange = 0.0f;
	yChange = 0.0f;
	esNoche = false;
	camaraActiva = 0;
	soltarCamara = false;
	mouseFirstMoved = true;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}
Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	mainWindow = NULL;
	bufferWidth = 0;
	bufferHeight = 0;
	lastX = 0.0f;
	lastY = 0.0f;
	xChange = 0.0f;
	yChange = 0.0f;
	esNoche = false;
	camaraActiva = 0;
	soltarCamara = false;
	mouseFirstMoved = true;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}
int Window::Initialise()
{
	//iniciar GLFW
	if (!glfwInit())
	{
		printf("Fallo al inicializar GLFW\n");
		glfwTerminate();
		return 1;
	}
	//se pide la version 4.3 de OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//core profile, solo deja usar OpenGL moderno sin las funciones viejas
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//crear la ventana, el texto es su titulo
	mainWindow = glfwCreateWindow(width, height, "Proyecto Final", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo al crear la ventana con GLFW\n");
		glfwTerminate();
		return 1;
	}
	//medidas reales del area donde se dibuja
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//le dice a OpenGL que se va a dibujar en esta ventana
	glfwMakeContextCurrent(mainWindow);

	//teclado y mouse
	createCallbacks();

	//permitir extensiones nuevas
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Fallo al inicializar GLEW\n");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST); //buffer de profundidad, para que lo de atras no tape lo de enfrente

	glViewport(0, 0, bufferWidth, bufferHeight);
	//guarda esta clase dentro de la ventana para poder usarla en los callbacks, que son static
	glfwSetWindowUserPointer(mainWindow, this);

	return 0;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

//regresan cuanto se movio el mouse desde la ultima vez y lo reinician en 0
GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

//regresa si se presiono F y lo reinicia en false, asi la camara solo se suelta una vez por tecla
bool Window::getsoltarCamara()
{
	bool soltar = soltarCamara;
	soltarCamara = false;
	return soltar;
}

//se llama una vez por cada evento de teclado, ya sea al presionar, mantener o soltar una tecla
void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	//N alterna entre atardecer y noche, solo al presionar para que no parpadee si se deja apretada
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		theWindow->esNoche = !theWindow->esNoche;
	}
	//C pasa a la siguiente camara y despues de la ultima regresa a la primera
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = (theWindow->camaraActiva + 1) % 5;
	}
	//las teclas 1 a 5 van directo a cada camara, la 1 es la libre
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 0;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 1;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 2;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 3;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 4;
	}
	//F suelta la camara fija, se pasa a la libre y en main.cpp la libre toma el lugar de la fija
	if (key == GLFW_KEY_F && action == GLFW_PRESS && theWindow->camaraActiva != 0)
	{
		theWindow->soltarCamara = true;
		theWindow->camaraActiva = 0;
	}

	//guarda que teclas estan presionadas, la camara y el muneco lo leen con getsKeys()
	//las teclas WASD ya las usa la camara
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	//la primera vez no hay posicion anterior, asi que se toma la actual para que la camara no brinque
	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}


Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();

}
