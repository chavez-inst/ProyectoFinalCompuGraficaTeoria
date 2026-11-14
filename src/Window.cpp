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
	desfilePausado = false;
	reiniciarDesfile = false;
	sinSonido = false;
	usarCarta = false;
	verTablero = false;
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
	desfilePausado = false;
	reiniciarDesfile = false;
	sinSonido = false;
	usarCarta = false;
	verTablero = false;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//core profile, sin las funciones viejas
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//ventana maximizada del tamano del monitor, con su barra de titulo para poder
	//cambiar de ventana y tomar capturas; no se deja cambiar de tamano porque
	//la proyeccion se calcula una sola vez al inicio
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* modo = glfwGetVideoMode(monitor);
	width = modo->width;
	height = modo->height;
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	mainWindow = glfwCreateWindow(width, height, "Proyecto Final", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo al crear la ventana con GLFW\n");
		glfwTerminate();
		return 1;
	}
	//ya maximizada se le quita lo ajustable, asi respeta la barra de tareas
	glfwSetWindowAttrib(mainWindow, GLFW_RESIZABLE, GLFW_FALSE);
	//tamano real del area de dibujo
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	glfwMakeContextCurrent(mainWindow);

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

	glEnable(GL_DEPTH_TEST); //lo de atras no tapa lo de enfrente

	glViewport(0, 0, bufferWidth, bufferHeight);
	//los callbacks son static, por eso se guarda la clase aqui
	glfwSetWindowUserPointer(mainWindow, this);

	return 0;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

//regresan cuanto se movio el mouse y lo reinician
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

//regresa si se presiono F y lo reinicia
bool Window::getsoltarCamara()
{
	bool soltar = soltarCamara;
	soltarCamara = false;
	return soltar;
}

//regresa si se presiono R y lo reinicia
bool Window::getreiniciarDesfile()
{
	bool reiniciar = reiniciarDesfile;
	reiniciarDesfile = false;
	return reiniciar;
}

//true solo en el frame en que se presiono la I
bool Window::getusarCarta()
{
	bool usar = usarCarta;
	usarCarta = false;
	return usar;
}

//se llama en cada evento de teclado, al presionar, mantener o soltar
void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	//solo al presionar, para que no parpadee si se deja apretada
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		theWindow->esNoche = !theWindow->esNoche;
	}
	//vuelve a la primera despues de la ultima
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = (theWindow->camaraActiva + 1) % 5;
	}
	//van directo a cada camara
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
	//se pasa a la libre, que en main.cpp toma el lugar de la fija
	if (key == GLFW_KEY_F && action == GLFW_PRESS && theWindow->camaraActiva != 0)
	{
		theWindow->soltarCamara = true;
		theWindow->camaraActiva = 0;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		theWindow->desfilePausado = !theWindow->desfilePausado;
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		theWindow->reiniciarDesfile = true;
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		theWindow->sinSonido = !theWindow->sinSonido;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		theWindow->usarCarta = true;
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		theWindow->verTablero = !theWindow->verTablero;
	}

	//guarda las teclas presionadas, la camara y el muneco las leen con getsKeys()
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

	//la primera vez se toma la posicion actual para que la camara no brinque
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
