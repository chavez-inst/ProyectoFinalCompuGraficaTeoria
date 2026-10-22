/*
Proyecto Final - Computacion Grafica
Calenda de Oaxaca frente al Templo de Santo Domingo
Escenario: plaza, calle del circuito, bloques provisionales de los edificios y un muneco de cubos jerarquico
Controles: WASD y mouse mueven la camara libre, C cambia de camara, N cambia entre atardecer y noche, ESC cierra
Teclas de prueba del muneco: Y y U giran los hombros, H y J los codos, K y L las caderas, O y P las rodillas, Z y X la cabeza

Partes de este archivo, cada una empieza con un titulo entre signos =
 1. variables globales de texturas, modelos, materiales y luces
 2. geometria hecha a mano del piso, el plano de vegetacion, el cubo y la calle del circuito
 3. personaje jerarquico con su pose, las medidas del esqueleto, el dibujo y las teclas de prueba
 4. main, donde se cargan las texturas, modelos, skybox, materiales y luces
 5. while, donde se dibuja todo cada frame
*/

//esto solo va aqui y en ningun otro archivo, porque si no da error de simbolos repetidos
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
std::vector<MeshModel*> meshListModel; //mallas hechas a mano, cada vertice lleva posicion, coordenadas de textura y normal
std::vector<Shader> shaderList;

//de las tres camaras solo la libre, que es camera, se mueve con teclado y mouse, y las otras dos son fijas
Camera camera;
Camera camaraFachada;
Camera camaraDesfile;

//texturas, sus archivos van en la carpeta Textures
Texture plazaTexture;
Texture calleTexture;
Texture plainTexture; //textura blanca, con ella el objeto se ve solo con el color que se le pase

//un cielo para la tarde y otro para la noche, se dibuja uno u otro segun la tecla N
Skybox skyboxTarde;
Skybox skyboxNoche;

//materiales
Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

//luz direccional que hace de sol, hay una de tarde y otra de noche y al shader se manda solo una segun la tecla N
DirectionalLight luzTarde;
DirectionalLight luzNoche;
//arreglos de luces, el maximo de cada una esta en CommonValues.h
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

//shaders que usan todos los objetos menos el skybox, que carga los suyos en Skybox.cpp
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";


// ================== 2. GEOMETRIA HECHA A MANO ==================

//calcula las normales promediando las de los triangulos que comparten cada vertice
//se llama antes de CreateMeshModel porque ahi es donde se copian los vertices a la GPU
//la normal sale segun el orden de los indices, y para que apunte hacia afuera del objeto
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

	//plaza de 120 por 80 metros, con las normales hacia arriba, que es hacia afuera del piso
	//la imagen se repite cada 4 metros, por eso u va de 0 a 30 y v de 0 a 20
	GLfloat floorVertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-60.0f, 0.0f, -40.0f,	0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		60.0f, 0.0f, -40.0f,	30.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-60.0f, 0.0f, 40.0f,	0.0f, 20.0f,	0.0f, 1.0f, 0.0f,
		60.0f, 0.0f, 40.0f,		30.0f, 20.0f,	0.0f, 1.0f, 0.0f
	};

	//dos planos cruzados que sirven para plantas o cosas con transparencia, se van a usar para los arboles
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

	//CreateMeshModel recibe los vertices, los indices, cuantos floats tiene el arreglo y cuantos indices son
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

		// back, la S va al reves para que la imagen no salga en espejo
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


//medidas del circuito, que son dos rectas paralelas al eje X unidas por dos semicirculos
const float radioCircuito = 15.0f;		//radio de las curvas, medido al centro de la calle
const float mitadRecta = 25.0f;			//las rectas van de -25 a 25 en x, y en esos puntos estan los centros de las curvas
const float anchoCalle = 6.0f;

//agrega los dos vertices de un punto del centro de la calle, uno en el borde interior y otro en el exterior
//dx y dz forman la direccion que va del centro de la calle hacia el borde exterior
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

//arma la calle del circuito como una sola malla, recorre el centro de la calle y en cada punto agrega sus dos bordes
void CrearCalle()
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	const int segmentos = 24; //tramos rectos con los que se aproxima cada semicirculo
	float largoTramo = radioCircuito * 180.0f / segmentos * toRadians; //largo de arco de cada tramo
	float recorrido = 0.0f;

	//recta sur, del lado de Z positivo, de oeste a este
	AgregarPuntoCalle(vertices, -mitadRecta, radioCircuito, 0.0f, 1.0f, recorrido);
	recorrido += 2.0f * mitadRecta;
	AgregarPuntoCalle(vertices, mitadRecta, radioCircuito, 0.0f, 1.0f, recorrido);

	//curva este, donde el angulo va de 0 a 180 grados alrededor del centro (25, 0)
	for (int i = 1; i <= segmentos; i++)
	{
		float angulo = i * 180.0f / segmentos * toRadians;
		float dx = sin(angulo);
		float dz = cos(angulo);
		recorrido += largoTramo;
		AgregarPuntoCalle(vertices, mitadRecta + radioCircuito * dx, radioCircuito * dz, dx, dz, recorrido);
	}

	//recta norte, del lado de Z negativo, de este a oeste
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
	//los vertices pares son del borde interior y los impares del exterior
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


//dibuja el cubo como un bloque de color, donde posicion es el centro de su base y tamano es el ancho, alto y fondo en metros
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


// ================== 3. PERSONAJE JERARQUICO ==================
//cada parte se dibuja a partir de la matriz de su padre. Del cuerpo salen la cabeza, los brazos y las piernas, del brazo sale el antebrazo y del muslo la pierna
//asi al girar el hombro tambien se mueven el antebrazo y la mano

//angulos de las articulaciones, en grados
//el hombro gira hacia adelante sobre X, donde un angulo negativo sube el brazo al frente, y hacia el lado sobre Z, donde uno positivo mueve la mano hacia X positivo
//codos, caderas y rodillas giran sobre X. Con un angulo negativo el codo dobla hacia el frente y con uno positivo la rodilla dobla hacia atras
//la cabeza gira sobre Y para voltear
struct PosePersona
{
	float hombroIzqAdelante, hombroIzqLado, codoIzq;
	float hombroDerAdelante, hombroDerLado, codoDer;
	float caderaIzq, rodillaIzq;
	float caderaDer, rodillaDer;
	float cabeza;
};

//variables de cada articulacion, la animacion las escribe y el dibujo solo las lee, asi despues las pueden controlar los KeyFrames
//van en el mismo orden que en PosePersona, primero el brazo izquierdo, luego el derecho, despues las piernas y al final la cabeza
PosePersona poseMuneco = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

//medidas del esqueleto en metros, los modelos que reemplacen a los cubos deben respetarlas
//y tener su origen en la articulacion alrededor de la que giran
const float alturaCadera = 0.92f;							//del piso al origen del cuerpo
const glm::vec3 posHombro = glm::vec3(0.2f, 0.47f, 0.0f);	//hombro izquierdo, el derecho usa la misma posicion con x negativa
const glm::vec3 posCuello = glm::vec3(0.0f, 0.55f, 0.0f);
const glm::vec3 posCadera = glm::vec3(0.095f, -0.02f, 0.0f);	//cadera izquierda
const float largoBrazo = 0.28f;		//del hombro al codo
const float largoAntebrazo = 0.29f;	//del codo a la mano
const float largoMuslo = 0.44f;		//de la cadera a la rodilla
const float largoPierna = 0.46f;	//de la rodilla a la planta del pie

//dibuja una parte del muneco con el cubo
//articulacion es la matriz del punto donde gira la parte y centro es donde queda el centro del cubo respecto a ese punto
//la escala solo se aplica a esta copia de la matriz, asi no deforma a las partes hijas
void DibujarParte(glm::mat4 articulacion, glm::vec3 centro, glm::vec3 tamano, glm::vec3 color, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model = glm::translate(articulacion, centro);
	model = glm::scale(model, tamano);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshListModel[2]->RenderMeshModel();
}

//dibuja un brazo o una pierna de dos partes, la primera gira en la articulacion y la segunda cuelga de ella y se dobla en el codo o la rodilla
//las dos partes cuelgan hacia Y negativo desde su articulacion
void DibujarExtremidad(glm::mat4 padre, glm::vec3 articulacion, float adelante, float lado,
	float largo1, float grosor1, glm::vec3 color1, float doblez, float largo2, float grosor2, glm::vec3 color2,
	GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model = glm::translate(padre, articulacion);
	model = glm::rotate(model, adelante * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, lado * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	DibujarParte(model, glm::vec3(0.0f, -largo1 / 2.0f, 0.0f), glm::vec3(grosor1, largo1, grosor1), color1, uniformModel, uniformColor);

	//el codo o la rodilla esta al final de la primera parte
	model = glm::translate(model, glm::vec3(0.0f, -largo1, 0.0f));
	model = glm::rotate(model, doblez * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	DibujarParte(model, glm::vec3(0.0f, -largo2 / 2.0f, 0.0f), glm::vec3(grosor2, largo2, grosor2), color2, uniformModel, uniformColor);
}

//base es la posicion y el giro del muneco sobre el piso, y el muneco mira hacia Z positivo de la base
void DibujarMuneco(glm::mat4 base, PosePersona pose, GLuint uniformModel, GLuint uniformColor)
{
	glm::vec3 colorCamisa = glm::vec3(0.80f, 0.20f, 0.20f);
	glm::vec3 colorPiel = glm::vec3(0.85f, 0.65f, 0.50f);
	glm::vec3 colorNariz = glm::vec3(0.60f, 0.40f, 0.30f);
	glm::vec3 colorPantalon = glm::vec3(0.20f, 0.25f, 0.55f);
	glm::vec3 colorEspinilla = glm::vec3(0.12f, 0.15f, 0.35f); //mas oscuro que el muslo para distinguir la rodilla

	//el cuerpo es la raiz de la jerarquia y su origen esta a la altura de la cadera
	glm::mat4 cuerpo = glm::translate(base, glm::vec3(0.0f, alturaCadera, 0.0f));
	DibujarParte(cuerpo, glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.30f, 0.60f, 0.20f), colorCamisa, uniformModel, uniformColor);

	//la cabeza gira en el cuello y la nariz sirve para ver hacia donde mira
	glm::mat4 cabeza = glm::translate(cuerpo, posCuello);
	cabeza = glm::rotate(cabeza, pose.cabeza * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	DibujarParte(cabeza, glm::vec3(0.0f, 0.18f, 0.0f), glm::vec3(0.22f, 0.26f, 0.24f), colorPiel, uniformModel, uniformColor);
	DibujarParte(cabeza, glm::vec3(0.0f, 0.16f, 0.13f), glm::vec3(0.05f, 0.05f, 0.05f), colorNariz, uniformModel, uniformColor);

	//brazos que van del hombro al codo y del codo a la mano, el derecho usa la x del hombro en negativo
	glm::vec3 hombroDer = glm::vec3(-posHombro.x, posHombro.y, posHombro.z);
	DibujarExtremidad(cuerpo, posHombro, pose.hombroIzqAdelante, pose.hombroIzqLado,
		largoBrazo, 0.09f, colorCamisa, pose.codoIzq, largoAntebrazo, 0.08f, colorPiel, uniformModel, uniformColor);
	DibujarExtremidad(cuerpo, hombroDer, pose.hombroDerAdelante, pose.hombroDerLado,
		largoBrazo, 0.09f, colorCamisa, pose.codoDer, largoAntebrazo, 0.08f, colorPiel, uniformModel, uniformColor);

	//piernas que van de la cadera a la rodilla y de la rodilla al pie
	glm::vec3 caderaDer = glm::vec3(-posCadera.x, posCadera.y, posCadera.z);
	DibujarExtremidad(cuerpo, posCadera, pose.caderaIzq, 0.0f,
		largoMuslo, 0.13f, colorPantalon, pose.rodillaIzq, largoPierna, 0.11f, colorEspinilla, uniformModel, uniformColor);
	DibujarExtremidad(cuerpo, caderaDer, pose.caderaDer, 0.0f,
		largoMuslo, 0.13f, colorPantalon, pose.rodillaDer, largoPierna, 0.11f, colorEspinilla, uniformModel, uniformColor);
}

//teclas de prueba para girar las articulaciones del muneco, se quitan cuando llegue la animacion
//esta es la parte que escribe la pose, y el dibujo en el while solo la lee
//Y y U giran los hombros, H y J los codos, K y L las caderas, O y P las rodillas, Z y X la cabeza
void ProbarArticulaciones(bool* keys, GLfloat deltaTime)
{
	float paso = 1.5f * deltaTime; //grados que gira en este frame mientras la tecla esta presionada

	//en hombros y caderas cada lado gira al reves que el otro, como al caminar
	if (keys[GLFW_KEY_Y])
	{
		poseMuneco.hombroIzqAdelante -= paso;
		poseMuneco.hombroDerAdelante += paso;
	}
	if (keys[GLFW_KEY_U])
	{
		poseMuneco.hombroIzqAdelante += paso;
		poseMuneco.hombroDerAdelante -= paso;
	}
	if (keys[GLFW_KEY_K])
	{
		poseMuneco.caderaIzq -= paso;
		poseMuneco.caderaDer += paso;
	}
	if (keys[GLFW_KEY_L])
	{
		poseMuneco.caderaIzq += paso;
		poseMuneco.caderaDer -= paso;
	}
	//en codos y rodillas los dos lados doblan igual
	if (keys[GLFW_KEY_H])
	{
		poseMuneco.codoIzq -= paso;
		poseMuneco.codoDer -= paso;
	}
	if (keys[GLFW_KEY_J])
	{
		poseMuneco.codoIzq += paso;
		poseMuneco.codoDer += paso;
	}
	if (keys[GLFW_KEY_O])
	{
		poseMuneco.rodillaIzq += paso;
		poseMuneco.rodillaDer += paso;
	}
	if (keys[GLFW_KEY_P])
	{
		poseMuneco.rodillaIzq -= paso;
		poseMuneco.rodillaDer -= paso;
	}
	if (keys[GLFW_KEY_Z])
	{
		poseMuneco.cabeza += paso;
	}
	if (keys[GLFW_KEY_X])
	{
		poseMuneco.cabeza -= paso;
	}

	//limites para que ninguna articulacion se doble hacia donde no puede una persona
	poseMuneco.hombroIzqAdelante = glm::clamp(poseMuneco.hombroIzqAdelante, -90.0f, 90.0f);
	poseMuneco.hombroDerAdelante = glm::clamp(poseMuneco.hombroDerAdelante, -90.0f, 90.0f);
	poseMuneco.codoIzq = glm::clamp(poseMuneco.codoIzq, -140.0f, 0.0f);
	poseMuneco.codoDer = glm::clamp(poseMuneco.codoDer, -140.0f, 0.0f);
	poseMuneco.caderaIzq = glm::clamp(poseMuneco.caderaIzq, -60.0f, 60.0f);
	poseMuneco.caderaDer = glm::clamp(poseMuneco.caderaDer, -60.0f, 60.0f);
	poseMuneco.rodillaIzq = glm::clamp(poseMuneco.rodillaIzq, 0.0f, 130.0f);
	poseMuneco.rodillaDer = glm::clamp(poseMuneco.rodillaDer, 0.0f, 130.0f);
	poseMuneco.cabeza = glm::clamp(poseMuneco.cabeza, -80.0f, 80.0f);
}


int main()
{
	mainWindow = Window(1366, 768); //tambien jala con 1280x1024 o 1024x768
	mainWindow.Initialise();
	CreateObjects();
	CrearCubo();
	CrearCalle();
	CreateShaders();

	//Camera recibe la posicion inicial, el vector up, yaw, pitch, la velocidad de movimiento y la velocidad de giro
	//la vista inicial esta al sur de la plaza a 25 m de altura, mirando hacia el circuito. Con yaw de -90 mira hacia Z negativo y con pitch de -25 se inclina hacia abajo
	camera = Camera(glm::vec3(0.0f, 25.0f, 60.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -25.0f, 0.3f, 0.5f);
	//la de la fachada esta en la plaza a 10 m de altura y con yaw de 0 mira hacia X positivo, asi se ven la curva este y el templo completo
	//velocidades en 0 porque esta camara no se mueve
	camaraFachada = Camera(glm::vec3(5.0f, 10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 2.0f, 0.0f, 0.0f);
	//la del desfile esta dentro del ovalo y con yaw de 90 mira hacia Z positivo, de lado hacia la recta sur
	camaraDesfile = Camera(glm::vec3(0.0f, 4.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -10.0f, 0.0f, 0.0f);
	//camara con la que se dibuja la escena, cambia en el while segun la tecla C
	Camera* camaraActual = &camera;

	// ================== 4. CARGA DE RECURSOS ==================

	//en las texturas primero se asigna la ruta y luego se carga
	//LoadTextureA es para imagenes con canal alfa como png o tga, y LoadTexture para las que no tienen, como jpg
	plazaTexture = Texture("Textures/plaza.png");
	plazaTexture.LoadTextureA();
	calleTexture = Texture("Textures/calle.png");
	calleTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();

	//las 6 caras del skybox van en este orden: rt, lf, dn, up, bk, ft
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

	//Material recibe la intensidad especular y el shininess
	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	// ---- luces ----
	//orden de los parametros de cada tipo de luz
	//DirectionalLight recibe el color r g b, la intensidad ambiental y la difusa, y la direccion x y z
	//PointLight recibe el color r g b, la intensidad ambiental y la difusa, la posicion x y z, y la atenuacion constante, lineal y exponencial
	//SpotLight recibe lo mismo que PointLight pero con la direccion x y z despues de la posicion, y al final el angulo del cono

	//siempre tiene que haber una luz direccional
	//de tarde es un sol naranja y bajo que viene del oeste, asi le pega de frente a la fachada de Santo Domingo
	luzTarde = DirectionalLight(1.0f, 0.75f, 0.5f,
		0.35f, 0.5f,
		1.0f, -0.4f, 0.3f);
	//de noche es una luz azul tenue casi desde arriba, como la luna
	luzNoche = DirectionalLight(0.4f, 0.45f, 0.7f,
		0.12f, 0.15f,
		0.2f, -1.0f, 0.3f);

	//farolas en la orilla exterior del circuito, 3 en cada recta y 1 en cada curva
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
	//contador de luces puntuales, en el while cambia a 0 de tarde y a numFarolas de noche
	unsigned int pointLightCount = 0;

	//contador de spotlights, no puede pasar de MAX_SPOT_LIGHTS
	unsigned int spotLightCount = 0;
	//luz de la fachada de Santo Domingo, sale del piso frente al templo y apunta hacia arriba y hacia la fachada
	spotLights[0] = SpotLight(1.0f, 0.9f, 0.75f,
		0.0f, 2.0f,
		32.0f, 0.5f, 0.0f,
		1.0f, 0.7f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		45.0f);
	spotLightCount++;

	//luz del kiosco, apunta de arriba hacia abajo y el cono de 30 grados cubre toda su base
	spotLights[1] = SpotLight(1.0f, 0.85f, 0.6f,
		0.0f, 1.5f,
		-25.0f, 12.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		30.0f);
	spotLightCount++;
	//para agregar otra luz se pone en el siguiente lugar libre del arreglo y se suma 1 al contador

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;

	//casas provisionales, de cada una se guarda el centro de su base, su ancho, alto y fondo, y su color
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

	//ojo, glm::perspective usa radianes, asi que el 45.0f no son 45 grados, pero asi venia del lab y se ve bien
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// ================== 5. CICLO PRINCIPAL ==================
	//todo lo que esta dentro del while se repite cada frame hasta que se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//eventos de teclado y mouse
		glfwPollEvents();

		//el cambio del mouse se lee siempre, aunque no se use, para que no se acumule mientras hay una camara fija
		GLfloat cambioX = mainWindow.getXChange();
		GLfloat cambioY = mainWindow.getYChange();
		if (mainWindow.getcamaraActiva() == 1)
		{
			camaraActual = &camaraFachada;
		}
		else if (mainWindow.getcamaraActiva() == 2)
		{
			camaraActual = &camaraDesfile;
		}
		else
		{
			camaraActual = &camera;
			camera.keyControl(mainWindow.getsKeys(), deltaTime);
			camera.mouseControl(cambioX, cambioY);
		}

		//en la animacion se escriben las variables de la pose, y el dibujo de abajo solo las lee
		ProbarArticulaciones(mainWindow.getsKeys(), deltaTime);

		//limpiar la pantalla
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//el skybox va primero, antes que todo lo demas
		if (mainWindow.getesNoche())
		{
			skyboxNoche.DrawSkybox(camaraActual->calculateViewMatrix(), projection);
		}
		else
		{
			skyboxTarde.DrawSkybox(camaraActual->calculateViewMatrix(), projection);
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
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camaraActual->calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camaraActual->getCameraPosition().x, camaraActual->getCameraPosition().y, camaraActual->getCameraPosition().z);

		//aqui en el while es donde se pueden cambiar cosas de las luces mientras corre el programa
		//con la tecla N, de noche cambia la luz direccional y se encienden las farolas
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

		//para dibujar cada objeto siempre se pone la matriz model, el color, la textura y el material, y luego se hace el render
		glm::mat4 model(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //con blanco la textura se ve con sus colores normales

		// ---------- objetos opacos ----------

		//plaza
		model = glm::mat4(1.0);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		plazaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[0]->RenderMeshModel();

		//la calle del circuito va 1 cm arriba de la plaza para que no parpadeen al estar a la misma altura
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.01f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		calleTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[3]->RenderMeshModel();

		//los bloques provisionales usan la textura blanca para que se vea solo el color de cada uno
		plainTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//Santo Domingo, con su nave y dos torres afuera de la curva este y la fachada hacia el circuito
		glm::vec3 colorCantera = glm::vec3(0.85f, 0.78f, 0.62f);
		DibujarBloque(glm::vec3(52.0f, 0.0f, 0.0f), glm::vec3(12.0f, 14.0f, 22.0f), colorCantera, uniformModel, uniformColor);
		DibujarBloque(glm::vec3(48.5f, 0.0f, -9.0f), glm::vec3(5.0f, 22.0f, 5.0f), colorCantera, uniformModel, uniformColor);
		DibujarBloque(glm::vec3(48.5f, 0.0f, 9.0f), glm::vec3(5.0f, 22.0f, 5.0f), colorCantera, uniformModel, uniformColor);

		//kiosco con su base y su cuerpo, dentro de la curva oeste
		DibujarBloque(glm::vec3(-25.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.5f, 10.0f), glm::vec3(0.75f, 0.75f, 0.70f), uniformModel, uniformColor);
		DibujarBloque(glm::vec3(-25.0f, 0.5f, 0.0f), glm::vec3(6.0f, 4.0f, 6.0f), glm::vec3(0.30f, 0.55f, 0.35f), uniformModel, uniformColor);

		//fuente en el centro de la plaza
		DibujarBloque(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(4.0f, 1.0f, 4.0f), glm::vec3(0.40f, 0.70f, 0.90f), uniformModel, uniformColor);

		//casas
		for (int i = 0; i < numCasas; i++)
		{
			DibujarBloque(posicionCasas[i], tamanoCasas[i], colorCasas[i], uniformModel, uniformColor);
		}

		//farolas provisionales con un poste delgado y la lampara arriba
		for (int i = 0; i < numFarolas; i++)
		{
			DibujarBloque(posicionFarolas[i], glm::vec3(0.2f, altoPoste, 0.2f), glm::vec3(0.25f, 0.25f, 0.25f), uniformModel, uniformColor);
			DibujarBloque(posicionFarolas[i] + glm::vec3(0.0f, altoPoste, 0.0f), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(1.0f, 0.9f, 0.5f), uniformModel, uniformColor);
		}

		//muneco de cubos en la recta sur, girado 90 grados para que mire hacia X positivo, que es hacia donde va a avanzar el desfile
		//se sube 1 cm igual que la calle para que los pies queden encima de ella
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-5.0f, 0.01f, 15.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarMuneco(model, poseMuneco, uniformModel, uniformColor);

		// ---------- objetos con transparencia ----------
		//van al final, despues de todos los opacos y con glEnable(GL_BLEND), por ahora no hay ninguno

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
