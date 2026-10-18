/*
Proyecto Final - Computacion Grafica
Calenda de Oaxaca frente al Templo de Santo Domingo
Escenario: plaza, calle del circuito y bloques provisionales de los edificios
Controles: WASD y mouse mueven la camara, N cambia entre atardecer y noche, ESC cierra

Como esta acomodado este archivo (busquen los bloques ===):
 1. variables globales (texturas, modelos, materiales, luces)
 2. geometria hecha a mano (piso, plano de vegetacion, cubo, calle del circuito)
 3. main: carga de texturas, modelos, skybox, materiales y luces
 4. while: aqui se dibuja todo cada frame
*/

//esto solo va aqui, en ningun otro archivo (si no da error de simbolos repetidos)
#define STB_IMAGE_IMPLEMENTATION

#include <vector>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

//iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// ================== 1. VARIABLES GLOBALES ==================

Window mainWindow;
std::vector<MeshModel*> meshListModel; //mallas hechas a mano, cada vertice es x y z, u v, nx ny nz
std::vector<Shader> shaderList;

Camera camera;

//texturas (los archivos van en la carpeta Textures)
Texture plazaTexture;
Texture calleTexture;
Texture plainTexture; //blanca: el objeto se ve solo con el color que se le pase

//un cielo para la tarde y otro para la noche, se dibuja uno u otro segun la tecla N
Skybox skyboxTarde;
Skybox skyboxNoche;

//materiales
Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

//luz direccional (el "sol"): una de tarde y otra de noche, al shader se manda solo una segun la tecla N
DirectionalLight luzTarde;
DirectionalLight luzNoche;
//arreglos de luces, el maximo de cada una esta en CommonValues.h
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

//shaders que usan todos los objetos (los del skybox los carga Skybox.cpp)
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";


// ================== 2. GEOMETRIA HECHA A MANO ==================

//calcula las normales promediando las de los triangulos que comparten cada vertice
//se llama antes de CreateMeshModel porque ahi es donde se copian los vertices a la GPU
//la normal sale segun el orden de los indices: para que apunte hacia afuera del objeto,
//los triangulos tienen que ir en sentido antihorario vistos desde afuera
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


//cada malla que se crea aqui se mete a meshListModel y luego se dibuja con su indice
void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	//plaza de 120 x 80 metros, normales hacia arriba (hacia afuera del piso)
	//la imagen se repite cada 4 metros (u de 0 a 30, v de 0 a 20)
	GLfloat floorVertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-60.0f, 0.0f, -40.0f,	0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		60.0f, 0.0f, -40.0f,	30.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-60.0f, 0.0f, 40.0f,	0.0f, 20.0f,	0.0f, 1.0f, 0.0f,
		60.0f, 0.0f, 40.0f,		30.0f, 20.0f,	0.0f, 1.0f, 0.0f
	};

	//dos planos cruzados, sirve para plantas o cosas con transparencia (se usara para los arboles)
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4, 5, 6,
	   4, 6, 7
	};

	//mismas normales que el piso para que se ilumine igual de los dos lados
	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
	};

	//CreateMeshModel(vertices, indices, cantidad de floats del arreglo, cantidad de indices)
	MeshModel* piso = new MeshModel();
	piso->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(piso); //meshListModel[0]

	MeshModel* vegetacion = new MeshModel();
	vegetacion->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(vegetacion); //meshListModel[1]
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1); //shaderList[0]
}


//cubo de 1x1x1 centrado en el origen, con la imagen completa en cada cara
void CrearCubo()
{
	//cada normal apunta hacia afuera de su cara, igual que en Blender
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		4, 5, 6,
		6, 7, 4,
		// back
		8, 9, 10,
		10, 11, 8,
		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,
	};

	GLfloat cubo_vertices[] = {
		//  x      y      z      S      T      NX     NY     NZ
		// front
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,

		// right
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,

		// back (la S va al reves para que la imagen no salga en espejo)
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,

		// left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

		// bottom
		-0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,

		// top
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
	};

	MeshModel* cubo = new MeshModel();
	cubo->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(cubo); //meshListModel[2]
}


//medidas del circuito: dos rectas paralelas al eje X unidas por dos semicirculos
const float radioCircuito = 15.0f;		//radio de las curvas, medido al centro de la calle
const float mitadRecta = 25.0f;			//las rectas van de x = -25 a x = 25, ahi estan los centros de las curvas
const float anchoCalle = 6.0f;

//agrega los dos vertices (borde interior y borde exterior) de un punto del centro de la calle
//(dx, dz) es la direccion que va del centro de la calle hacia el borde exterior
//recorrido son los metros avanzados desde el inicio, se usa como coordenada u para repetir el adoquin
void AgregarPuntoCalle(std::vector<GLfloat>& vertices, float x, float z, float dx, float dz, float recorrido)
{
	float mitad = anchoCalle / 2.0f;
	float u = recorrido / 2.0f; //la imagen se repite cada 2 metros

	//borde interior
	vertices.insert(vertices.end(), { x - dx * mitad, 0.0f, z - dz * mitad,		u, 0.0f,	0.0f, 1.0f, 0.0f });
	//borde exterior
	//v llega a 3 en los 6 metros de ancho, asi la imagen tambien se repite cada 2 metros a lo ancho
	vertices.insert(vertices.end(), { x + dx * mitad, 0.0f, z + dz * mitad,		u, anchoCalle / 2.0f,	0.0f, 1.0f, 0.0f });
}

//calle del circuito como una sola malla: se recorre el centro de la calle y en cada punto se agregan sus dos bordes
void CrearCalle()
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	const int segmentos = 24; //tramos rectos con los que se aproxima cada semicirculo
	float largoTramo = radioCircuito * 180.0f / segmentos * toRadians; //largo de arco de cada tramo
	float recorrido = 0.0f;

	//recta sur (z positivo), de oeste a este
	AgregarPuntoCalle(vertices, -mitadRecta, radioCircuito, 0.0f, 1.0f, recorrido);
	recorrido += 2.0f * mitadRecta;
	AgregarPuntoCalle(vertices, mitadRecta, radioCircuito, 0.0f, 1.0f, recorrido);

	//curva este: el angulo va de 0 a 180 grados alrededor del centro (25, 0)
	for (int i = 1; i <= segmentos; i++)
	{
		float angulo = i * 180.0f / segmentos * toRadians;
		float dx = sin(angulo);
		float dz = cos(angulo);
		recorrido += largoTramo;
		AgregarPuntoCalle(vertices, mitadRecta + radioCircuito * dx, radioCircuito * dz, dx, dz, recorrido);
	}

	//recta norte (z negativo), de este a oeste
	recorrido += 2.0f * mitadRecta;
	AgregarPuntoCalle(vertices, -mitadRecta, -radioCircuito, 0.0f, -1.0f, recorrido);

	//curva oeste alrededor del centro (-25, 0), termina donde empezo la recta sur
	for (int i = 1; i <= segmentos; i++)
	{
		float angulo = i * 180.0f / segmentos * toRadians;
		float dx = -sin(angulo);
		float dz = -cos(angulo);
		recorrido += largoTramo;
		AgregarPuntoCalle(vertices, -mitadRecta + radioCircuito * dx, radioCircuito * dz, dx, dz, recorrido);
	}

	//cada par de puntos seguidos forma un rectangulo de dos triangulos
	//vertices pares: borde interior, impares: borde exterior
	unsigned int numPuntos = vertices.size() / 16;
	for (unsigned int k = 0; k < numPuntos - 1; k++)
	{
		unsigned int interior = 2 * k;
		unsigned int exterior = 2 * k + 1;
		indices.insert(indices.end(), { interior, exterior, interior + 2 });
		indices.insert(indices.end(), { interior + 2, exterior, exterior + 2 });
	}

	MeshModel* calle = new MeshModel();
	calle->CreateMeshModel(vertices.data(), indices.data(), vertices.size(), indices.size());
	meshListModel.push_back(calle); //meshListModel[3]
}


//dibuja el cubo como un bloque de color: posicion es el centro de su base, tamano es ancho, alto y fondo en metros
//se sube la mitad de su alto para que quede apoyado sobre el piso
void DibujarBloque(glm::vec3 posicion, glm::vec3 tamano, glm::vec3 color, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model(1.0);
	model = glm::translate(model, posicion + glm::vec3(0.0f, tamano.y / 2.0f, 0.0f));
	model = glm::scale(model, tamano);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshListModel[2]->RenderMeshModel();
}


int main()
{
	mainWindow = Window(1366, 768); //tambien jala con 1280x1024 o 1024x768
	mainWindow.Initialise();
	CreateObjects();
	CrearCubo();
	CrearCalle();
	CreateShaders();

	//Camera(posicion inicial, vector up, yaw, pitch, velocidad de movimiento, velocidad de giro)
	//vista inicial: al sur de la plaza, a 25 m de altura, mirando hacia el circuito (yaw -90 mira hacia -Z, pitch -25 inclina hacia abajo)
	camera = Camera(glm::vec3(0.0f, 25.0f, 60.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -25.0f, 0.3f, 0.5f);

	// ================== 3. CARGA DE RECURSOS ==================

	//texturas: primero se asigna la ruta y luego se carga
	//LoadTextureA para imagenes con canal alfa (png, tga) y LoadTexture para las que no tienen (jpg)
	plazaTexture = Texture("Textures/plaza.png");
	plazaTexture.LoadTextureA();
	calleTexture = Texture("Textures/calle.png");
	calleTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();

	//skybox: las 6 caras van en este orden: rt, lf, dn, up, bk, ft
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/tarde_rt.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_lf.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_dn.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_up.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_bk.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_ft.jpg");
	skyboxTarde = Skybox(skyboxFaces);

	//se vacia la lista para reusarla con las caras de la noche
	skyboxFaces.clear();
	skyboxFaces.push_back("Textures/Skybox/noche_rt.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_lf.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_dn.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_up.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_bk.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_ft.jpg");
	skyboxNoche = Skybox(skyboxFaces);

	//materiales: Material(intensidad especular, shininess)
	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	// ---- luces ----
	//los parametros van en este orden:
	//DirectionalLight(r, g, b, intensidad ambiental, intensidad difusa, direccion x, y, z)
	//PointLight(r, g, b, ambiental, difusa, posicion x, y, z, constante, lineal, exponente)
	//SpotLight(r, g, b, ambiental, difusa, posicion x, y, z, direccion x, y, z, constante, lineal, exponente, angulo del cono)

	//luz direccional: siempre tiene que haber una
	//tarde: sol naranja y bajo que viene del oeste, asi le pega de frente a la fachada de Santo Domingo
	luzTarde = DirectionalLight(1.0f, 0.75f, 0.5f,
		0.35f, 0.5f,
		1.0f, -0.4f, 0.3f);
	//noche: luz azul tenue casi desde arriba, como la luna
	luzNoche = DirectionalLight(0.4f, 0.45f, 0.7f,
		0.12f, 0.15f,
		0.2f, -1.0f, 0.3f);

	//farolas en la orilla exterior del circuito: 3 en cada recta y 1 en cada curva
	//la calle esta a 15 m del centro y mide 6 de ancho, asi que su orilla exterior queda a 18 m
	const int numFarolas = 8; //no debe pasar de MAX_POINT_LIGHTS
	const float altoPoste = 4.5f;
	glm::vec3 posicionFarolas[numFarolas] = {
		glm::vec3(-25.0f, 0.0f, 19.0f), glm::vec3(0.0f, 0.0f, 19.0f), glm::vec3(25.0f, 0.0f, 19.0f),
		glm::vec3(-25.0f, 0.0f, -19.0f), glm::vec3(0.0f, 0.0f, -19.0f), glm::vec3(25.0f, 0.0f, -19.0f),
		glm::vec3(44.0f, 0.0f, 0.0f), glm::vec3(-44.0f, 0.0f, 0.0f)
	};
	//cada farola tiene una luz puntual amarilla justo debajo de su lampara
	//la atenuacion hace que alumbre un circulo de unos 6 m de radio alrededor del poste
	for (int i = 0; i < numFarolas; i++)
	{
		pointLights[i] = PointLight(1.0f, 0.8f, 0.5f,
			0.0f, 1.2f,
			posicionFarolas[i].x, altoPoste - 0.2f, posicionFarolas[i].z,
			1.0f, 0.09f, 0.032f);
	}
	//contador de luces puntuales, cambia en el while: 0 de tarde y numFarolas de noche
	unsigned int pointLightCount = 0;

	//contador de spotlights (no puede pasar de MAX_SPOT_LIGHTS)
	unsigned int spotLightCount = 0;
	//fachada de Santo Domingo: desde el piso frente al templo, apuntando hacia arriba y hacia la fachada
	spotLights[0] = SpotLight(1.0f, 0.9f, 0.75f,
		0.0f, 2.0f,
		32.0f, 0.5f, 0.0f,
		1.0f, 0.7f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		45.0f);
	spotLightCount++;

	//kiosco: desde arriba hacia abajo, el cono de 30 grados cubre toda su base
	spotLights[1] = SpotLight(1.0f, 0.85f, 0.6f,
		0.0f, 1.5f,
		-25.0f, 12.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		30.0f);
	spotLightCount++;
	//para agregar otra luz: se pone en el siguiente lugar libre del arreglo y se suma 1 al contador

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;

	//casas provisionales: centro de la base, tamano (ancho, alto, fondo) y color de cada una
	const int numCasas = 12;
	glm::vec3 posicionCasas[numCasas] = {
		//dentro del ovalo
		glm::vec3(-11.0f, 0.0f, 5.0f), glm::vec3(10.0f, 0.0f, -5.0f), glm::vec3(18.0f, 0.0f, 5.0f),
		//afuera de la recta sur
		glm::vec3(-35.0f, 0.0f, 27.0f), glm::vec3(-20.0f, 0.0f, 28.0f), glm::vec3(-5.0f, 0.0f, 27.0f),
		glm::vec3(10.0f, 0.0f, 28.0f), glm::vec3(25.0f, 0.0f, 27.0f),
		//afuera de la recta norte
		glm::vec3(-30.0f, 0.0f, -27.0f), glm::vec3(-12.0f, 0.0f, -28.0f), glm::vec3(6.0f, 0.0f, -27.0f),
		glm::vec3(22.0f, 0.0f, -28.0f)
	};
	glm::vec3 tamanoCasas[numCasas] = {
		glm::vec3(6.0f, 5.0f, 5.0f), glm::vec3(7.0f, 6.0f, 5.0f), glm::vec3(5.0f, 4.0f, 5.0f),
		glm::vec3(10.0f, 6.0f, 8.0f), glm::vec3(9.0f, 5.0f, 8.0f), glm::vec3(11.0f, 7.0f, 8.0f),
		glm::vec3(9.0f, 5.0f, 8.0f), glm::vec3(10.0f, 6.0f, 8.0f),
		glm::vec3(12.0f, 6.0f, 8.0f), glm::vec3(10.0f, 5.0f, 8.0f), glm::vec3(11.0f, 7.0f, 8.0f),
		glm::vec3(9.0f, 5.0f, 8.0f)
	};
	//colores de fachadas como los del centro de Oaxaca
	glm::vec3 colorCasas[numCasas] = {
		glm::vec3(0.95f, 0.75f, 0.30f), glm::vec3(0.80f, 0.35f, 0.30f), glm::vec3(0.40f, 0.60f, 0.80f),
		glm::vec3(0.90f, 0.55f, 0.65f), glm::vec3(0.95f, 0.85f, 0.55f), glm::vec3(0.45f, 0.70f, 0.50f),
		glm::vec3(0.85f, 0.45f, 0.25f), glm::vec3(0.60f, 0.45f, 0.75f),
		glm::vec3(0.40f, 0.60f, 0.80f), glm::vec3(0.95f, 0.75f, 0.30f), glm::vec3(0.80f, 0.35f, 0.30f),
		glm::vec3(0.90f, 0.55f, 0.65f)
	};

	//ojo: glm::perspective usa radianes, asi que el 45.0f no son 45 grados, pero asi venia del lab y se ve bien
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// ================== 4. CICLO PRINCIPAL ==================
	//todo lo que esta dentro del while se repite cada frame hasta que se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//eventos de teclado y mouse
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//limpiar la pantalla
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//el skybox va primero, antes que todo lo demas
		if (mainWindow.getesNoche())
		{
			skyboxNoche.DrawSkybox(camera.calculateViewMatrix(), projection);
		}
		else
		{
			skyboxTarde.DrawSkybox(camera.calculateViewMatrix(), projection);
		}

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();

		//intensidad especular y brillo del material
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		//aqui en el while es donde se pueden cambiar cosas de las luces mientras corre el programa
		//segun la tecla N: de noche cambia la luz direccional y se encienden las farolas
		if (mainWindow.getesNoche())
		{
			shaderList[0].SetDirectionalLight(&luzNoche);
			pointLightCount = numFarolas;
		}
		else
		{
			shaderList[0].SetDirectionalLight(&luzTarde);
			pointLightCount = 0;
		}

		//mandar todas las luces al shader
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		//para dibujar cada objeto siempre es lo mismo: matriz model, color, textura, material y render
		glm::mat4 model(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //blanco = la textura se ve con sus colores normales

		// ---------- objetos opacos ----------

		//plaza
		model = glm::mat4(1.0);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		plazaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[0]->RenderMeshModel();

		//calle del circuito: 1 cm arriba de la plaza para que no parpadeen al estar a la misma altura
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.01f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		calleTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[3]->RenderMeshModel();

		//bloques provisionales: textura blanca para que se vea solo el color de cada bloque
		plainTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//Santo Domingo: nave y dos torres, afuera de la curva este con la fachada hacia el circuito
		glm::vec3 colorCantera = glm::vec3(0.85f, 0.78f, 0.62f);
		DibujarBloque(glm::vec3(52.0f, 0.0f, 0.0f), glm::vec3(12.0f, 14.0f, 22.0f), colorCantera, uniformModel, uniformColor);
		DibujarBloque(glm::vec3(48.5f, 0.0f, -9.0f), glm::vec3(5.0f, 22.0f, 5.0f), colorCantera, uniformModel, uniformColor);
		DibujarBloque(glm::vec3(48.5f, 0.0f, 9.0f), glm::vec3(5.0f, 22.0f, 5.0f), colorCantera, uniformModel, uniformColor);

		//kiosco: base y cuerpo, dentro de la curva oeste
		DibujarBloque(glm::vec3(-25.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.5f, 10.0f), glm::vec3(0.75f, 0.75f, 0.70f), uniformModel, uniformColor);
		DibujarBloque(glm::vec3(-25.0f, 0.5f, 0.0f), glm::vec3(6.0f, 4.0f, 6.0f), glm::vec3(0.30f, 0.55f, 0.35f), uniformModel, uniformColor);

		//fuente en el centro de la plaza
		DibujarBloque(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(4.0f, 1.0f, 4.0f), glm::vec3(0.40f, 0.70f, 0.90f), uniformModel, uniformColor);

		//casas
		for (int i = 0; i < numCasas; i++)
		{
			DibujarBloque(posicionCasas[i], tamanoCasas[i], colorCasas[i], uniformModel, uniformColor);
		}

		//farolas provisionales: poste delgado y lampara arriba
		for (int i = 0; i < numFarolas; i++)
		{
			DibujarBloque(posicionFarolas[i], glm::vec3(0.2f, altoPoste, 0.2f), glm::vec3(0.25f, 0.25f, 0.25f), uniformModel, uniformColor);
			DibujarBloque(posicionFarolas[i] + glm::vec3(0.0f, altoPoste, 0.0f), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(1.0f, 0.9f, 0.5f), uniformModel, uniformColor);
		}

		// ---------- objetos con transparencia ----------
		//van al final, despues de todos los opacos, con glEnable(GL_BLEND) (todavia no hay ninguno)

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
