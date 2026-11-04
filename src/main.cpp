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
Texture andadorTexture;		//losas de cantera del andador
Texture empedradoTexture;	//calles que cruzan el andador
Texture banquetaTexture;	//banquetas de las manzanas
Texture plazaTexture;		//losas grandes del atrio, el jardin Labastida y el Zocalo
Texture plainTexture;		//textura blanca, con ella el objeto se ve solo con el color que se le pase
//fachadas de las casas, las de aplanado son claras para pintarlas con el color de cada casa
Texture fachadaCasa1Texture;	//casa de un piso
Texture fachadaCasa2Texture;	//casa de dos pisos con balcones
Texture fachadaCanteraTexture;	//edificio de cantera, no se pinta
Texture fachadaPortalTexture;	//portales del Zocalo con dos arcos abajo, ya viene pintada
//jardineras del Zocalo
Texture canteraTexture;			//cantera verde en bloques de las orillas, la imagen cubre 2 m
Texture tierraTexture;			//tierra roja de adentro, la imagen cubre 2 m
//tiene canal alfa y se dibuja al final con transparencia
Texture telonTexture;		//calle que sigue, al final de cada calle que cruza

//modelos, sus archivos van en la carpeta Models y sus texturas en Textures
Model farolaBrazo;		//farola con el farol colgado de un brazo, para el andador
Model farolaGlobos;		//farola de dos globos, para el Zocalo
Model santoDomingo;		//templo, torres y ex convento, con la fachada hacia Z positivo
Model sangreDeCristo;	//templo con su plataforma y escalones, con la fachada hacia Z positivo
Model kiosco;			//kiosco del Zocalo con su base de cantera, centrado en el origen
Model fuenteZocalo;		//fuente redonda con el agua y la escultura, centrada en el origen

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


//agrega a una malla un rectangulo acostado que va de (x0, z0) a (x1, z1) a la altura y, con la normal hacia arriba
//u y v salen de dividir x y z entre los metros que cubre la imagen, asi la textura queda pegada al mundo
//y dos pedazos vecinos siempre coinciden
void AgregarRectanguloPiso(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices,
	float x0, float z0, float x1, float z1, float y, float metrosImagen)
{
	unsigned int primero = (unsigned int)vertices.size() / 8; //cada vertice ocupa 8 floats
	vertices.insert(vertices.end(), {
		x0, y, z0,	x0 / metrosImagen, z0 / metrosImagen,	0.0f, 1.0f, 0.0f,
		x1, y, z0,	x1 / metrosImagen, z0 / metrosImagen,	0.0f, 1.0f, 0.0f,
		x0, y, z1,	x0 / metrosImagen, z1 / metrosImagen,	0.0f, 1.0f, 0.0f,
		x1, y, z1,	x1 / metrosImagen, z1 / metrosImagen,	0.0f, 1.0f, 0.0f });
	indices.insert(indices.end(), { primero, primero + 2, primero + 1, primero + 1, primero + 2, primero + 3 });
}

//agrega un rectangulo parado que va del punto (xa, za) al (xb, zb) y de la altura y0 a la y1
//nx y nz es la direccion hacia donde ve la cara, que debe ser hacia afuera del objeto
void AgregarRectanguloPared(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices,
	float xa, float za, float xb, float zb, float y0, float y1, float nx, float nz, float metrosImagen)
{
	unsigned int primero = (unsigned int)vertices.size() / 8;
	float u = glm::length(glm::vec2(xb - xa, zb - za)) / metrosImagen;
	float v0 = y0 / metrosImagen;
	float v1 = y1 / metrosImagen;
	vertices.insert(vertices.end(), {
		xa, y0, za,		0.0f, v0,	nx, 0.0f, nz,
		xb, y0, zb,		u, v0,		nx, 0.0f, nz,
		xb, y1, zb,		u, v1,		nx, 0.0f, nz,
		xa, y1, za,		0.0f, v1,	nx, 0.0f, nz });
	indices.insert(indices.end(), { primero, primero + 1, primero + 2, primero + 2, primero + 3, primero });
}

//bloque bajo con su tapa y sus cuatro lados, sirve para las banquetas y las plazas que estan un poco arriba de la calle
void AgregarPlataforma(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices,
	float x0, float z0, float x1, float z1, float alto, float metrosImagen)
{
	AgregarRectanguloPiso(vertices, indices, x0, z0, x1, z1, alto, metrosImagen);
	AgregarRectanguloPared(vertices, indices, x1, z0, x0, z0, 0.0f, alto, 0.0f, -1.0f, metrosImagen);	//lado norte
	AgregarRectanguloPared(vertices, indices, x0, z1, x1, z1, 0.0f, alto, 0.0f, 1.0f, metrosImagen);	//lado sur
	AgregarRectanguloPared(vertices, indices, x0, z0, x0, z1, 0.0f, alto, -1.0f, 0.0f, metrosImagen);	//lado poniente
	AgregarRectanguloPared(vertices, indices, x1, z1, x1, z0, 0.0f, alto, 1.0f, 0.0f, metrosImagen);	//lado oriente
}

//copia los vertices a la GPU como una malla nueva y la mete al final de meshListModel
void GuardarMalla(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices)
{
	MeshModel* malla = new MeshModel();
	malla->CreateMeshModel(vertices.data(), indices.data(), (unsigned int)vertices.size(), (unsigned int)indices.size());
	meshListModel.push_back(malla);
}

//cada malla que se crea aqui se mete a meshListModel y luego se dibuja con su indice
void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	//piso lejano de 1000 por 1000 metros, un poco abajo de las calles
	//llena todo lo que queda fuera de la ciudad, asi desde arriba nunca se ve la orilla del mundo
	GLfloat floorVertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-470.0f, -0.05f, -540.0f,	0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		530.0f, -0.05f, -540.0f,	1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		-470.0f, -0.05f, 460.0f,	0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		530.0f, -0.05f, 460.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f
	};

	//dos planos cruzados para plantas o cosas con transparencia
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


//cubo de lado 1 centrado en el origen, con la imagen completa en cada cara
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


//rectangulo parado de 1 por 1 con la base en el origen y la cara hacia Z positivo
//sirve para las fachadas, los telones y todo lo que es una imagen pegada en un plano
//se crea despues de los pisos de la ciudad, por eso queda en meshListModel[7]
void CrearPlano()
{
	unsigned int planoIndices[] = {
		0, 1, 2,
		2, 3, 0
	};
	GLfloat planoVertices[] = {
		-0.5f, 0.0f, 0.0f,	0.0f, 0.0f,		0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.0f,	1.0f, 0.0f,		0.0f, 0.0f, 1.0f,
		0.5f, 1.0f, 0.0f,	1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		-0.5f, 1.0f, 0.0f,	0.0f, 1.0f,		0.0f, 0.0f, 1.0f
	};
	MeshModel* plano = new MeshModel();
	plano->CreateMeshModel(planoVertices, planoIndices, 32, 6);
	meshListModel.push_back(plano); //meshListModel[7]
}


// ================== 3. TRAZADO DE LA ESCENA ==================
//1 unidad es 1 metro, el norte esta hacia Z negativo y el sur hacia Z positivo
//el andador va de norte a sur sobre x = 0, la calle mide 8 m, de -4 a 4, y cada banqueta 2 m

//un rectangulo en el piso, de la esquina noroeste (x0, z0) a la sureste (x1, z1)
struct Rectangulo
{
	float x0, z0, x1, z1;
};

const float altoBanqueta = 0.15f;

//manzanas, en cada una va una banqueta de 15 cm y las casas empiezan 2 m adentro de la orilla
const int numManzanas = 12;
const Rectangulo manzanas[numManzanas] = {
	{ -56.0f, -204.0f, 110.0f, -188.0f },	//fila al norte de Gurrion
	{ 56.0f, -180.0f, 110.0f, -108.0f },	//templo y ex convento de Santo Domingo
	{ -56.0f, -180.0f, -4.0f, -84.0f },		//poniente del atrio y del primer tramo, con la Sangre de Cristo
	{ -56.0f, -76.0f, -4.0f, -24.0f },		//poniente del segundo tramo
	{ -56.0f, -16.0f, -4.0f, 32.0f },		//poniente del tercer tramo
	{ 4.0f, -108.0f, 70.0f, -84.0f },		//oriente del primer tramo
	{ 40.0f, -76.0f, 70.0f, -46.0f },		//junto al jardin Labastida
	{ 4.0f, -46.0f, 70.0f, -24.0f },		//oriente del segundo tramo
	{ 4.0f, -16.0f, 70.0f, 32.0f },			//oriente del tercer tramo
	{ -56.0f, 42.0f, -35.0f, 112.0f },		//portales del poniente del Zocalo
	{ 35.0f, 42.0f, 70.0f, 112.0f },		//portales del oriente del Zocalo
	{ -56.0f, 112.0f, 70.0f, 128.0f }		//portales del sur del Zocalo
};

//plazas con losas grandes, tambien 15 cm arriba de la calle
const int numPlazas = 3;
const Rectangulo plazas[numPlazas] = {
	{ 4.0f, -180.0f, 56.0f, -108.0f },		//atrio de Santo Domingo, tan largo como el frente del templo
	{ 4.0f, -76.0f, 40.0f, -46.0f },		//jardin Labastida
	{ -35.0f, 42.0f, 35.0f, 112.0f }		//Zocalo
};

//tramos del andador entre las calles que cruzan, en los cruces se ve el empedrado de la calle
const int numTramos = 3;
const Rectangulo tramosAndador[numTramos] = {
	{ -4.0f, -180.0f, 4.0f, -84.0f },		//de Gurrion a Abasolo
	{ -4.0f, -76.0f, 4.0f, -24.0f },		//de Abasolo a Morelos
	{ -4.0f, -16.0f, 4.0f, 32.0f }			//de Morelos a Independencia
};

//jardineras del Zocalo en las cuatro esquinas alrededor del kiosco, dejan libres los pasillos hacia el kiosco y la fuente
const int numJardineras = 4;
const Rectangulo jardineras[numJardineras] = {
	{ -29.0f, 49.0f, -11.0f, 63.0f }, { 11.0f, 49.0f, 29.0f, 63.0f },
	{ -29.0f, 91.0f, -11.0f, 105.0f }, { 11.0f, 91.0f, 29.0f, 105.0f }
};
const float altoJardinera = 0.6f;	//lo que sobresalen del piso de la plaza
const float orillaJardinera = 0.5f;	//ancho del borde de cantera que rodea la tierra

//arma las mallas de las calles, el andador, las banquetas y las plazas con los rectangulos de arriba
void CrearEscenario()
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	//todo el piso de la ciudad es empedrado, la imagen cubre 2 m
	//el segundo rectangulo es el pedazo que queda detras de Santo Domingo
	AgregarRectanguloPiso(vertices, indices, -58.0f, -206.0f, 72.0f, 130.0f, 0.0f, 2.0f);
	AgregarRectanguloPiso(vertices, indices, 72.0f, -206.0f, 112.0f, -106.0f, 0.0f, 2.0f);
	GuardarMalla(vertices, indices); //meshListModel[3]

	//el andador va 1 cm arriba del empedrado para que no parpadeen al estar a la misma altura
	vertices.clear();
	indices.clear();
	for (int i = 0; i < numTramos; i++)
	{
		AgregarRectanguloPiso(vertices, indices, tramosAndador[i].x0, tramosAndador[i].z0, tramosAndador[i].x1, tramosAndador[i].z1, 0.01f, 2.0f);
	}
	GuardarMalla(vertices, indices); //meshListModel[4]

	vertices.clear();
	indices.clear();
	for (int i = 0; i < numManzanas; i++)
	{
		AgregarPlataforma(vertices, indices, manzanas[i].x0, manzanas[i].z0, manzanas[i].x1, manzanas[i].z1, altoBanqueta, 2.0f);
	}
	GuardarMalla(vertices, indices); //meshListModel[5]

	//en las plazas la imagen cubre 4 m porque las losas son mas grandes
	vertices.clear();
	indices.clear();
	for (int i = 0; i < numPlazas; i++)
	{
		AgregarPlataforma(vertices, indices, plazas[i].x0, plazas[i].z0, plazas[i].x1, plazas[i].z1, altoBanqueta, 4.0f);
	}
	GuardarMalla(vertices, indices); //meshListModel[6]
}

//jardineras del Zocalo, una malla con los bloques de cantera y otra con la tierra de encima
//los lados del bloque empiezan en 0, pero lo que queda abajo de la plaza no se ve
//la tierra va 1 cm arriba de la cantera para que no parpadeen
//se crean despues del plano, por eso quedan en meshListModel[8] y [9]
void CrearJardineras()
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	for (int i = 0; i < numJardineras; i++)
	{
		AgregarPlataforma(vertices, indices, jardineras[i].x0, jardineras[i].z0, jardineras[i].x1, jardineras[i].z1,
			altoBanqueta + altoJardinera, 2.0f);
	}
	GuardarMalla(vertices, indices); //meshListModel[8]

	vertices.clear();
	indices.clear();
	for (int i = 0; i < numJardineras; i++)
	{
		AgregarRectanguloPiso(vertices, indices, jardineras[i].x0 + orillaJardinera, jardineras[i].z0 + orillaJardinera,
			jardineras[i].x1 - orillaJardinera, jardineras[i].z1 - orillaJardinera, altoBanqueta + altoJardinera + 0.01f, 2.0f);
	}
	GuardarMalla(vertices, indices); //meshListModel[9]
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

//dibuja un plano parado, posicion es el centro de su orilla de abajo y giro son los grados sobre Y
//con giro 0 la imagen ve hacia Z positivo, con 90 hacia X positivo, con -90 hacia X negativo y con 180 hacia Z negativo
void DibujarPlano(glm::vec3 posicion, float giro, float ancho, float alto, glm::vec3 color, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model(1.0);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(ancho, alto, 1.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshListModel[7]->RenderMeshModel();
}


//filas de casas, cada casa es un cubo de color con su fachada en un plano 2 cm enfrente
//las medidas y los colores se van turnando para que no haya dos casas seguidas iguales
const int numModelosCasa = 7;
const float anchosCasa[numModelosCasa] = { 8.0f, 6.5f, 10.0f, 7.0f, 9.0f, 6.0f, 11.0f };
const float altosCasa[numModelosCasa] = { 6.0f, 7.5f, 5.5f, 8.0f, 6.5f, 7.2f, 5.8f };
const int numColoresCasa = 8;
//colores de fachadas como los del centro de Oaxaca, la textura clara de la fachada se pinta con ellos
const glm::vec3 coloresCasa[numColoresCasa] = {
	glm::vec3(0.95f, 0.78f, 0.35f), glm::vec3(0.80f, 0.32f, 0.28f), glm::vec3(0.38f, 0.56f, 0.88f),
	glm::vec3(0.93f, 0.64f, 0.68f), glm::vec3(0.96f, 0.94f, 0.88f), glm::vec3(0.88f, 0.58f, 0.30f),
	glm::vec3(0.58f, 0.76f, 0.56f), glm::vec3(0.92f, 0.56f, 0.46f)
};

//estilos de las filas
const int estiloColores = 0;	//casas de uno o dos pisos pintadas de colores
const int estiloCantera = 1;	//edificios de cantera de dos pisos, como los del tercer tramo
const int estiloPortal = 2;		//portales del Zocalo, todos del mismo ancho y alto

//inicio es la esquina donde empieza la fila al pie de la fachada, y giro es hacia donde ven las fachadas
//con giro 0 ven hacia Z positivo, con 90 hacia X positivo, con -90 hacia X negativo y con 180 hacia Z negativo
//la fila avanza hacia la derecha de quien ve las fachadas de frente, largo y fondo van en metros
//primera escoge con que medida y color empieza, asi cada fila se ve distinta
void DibujarFilaCasas(glm::vec3 inicio, float giro, float largo, float fondo, int estilo, int primera,
	GLuint uniformModel, GLuint uniformColor)
{
	//direccion hacia la calle y direccion en la que avanza la fila
	glm::vec3 haciaLaCalle = glm::vec3(sin(giro * toRadians), 0.0f, cos(giro * toRadians));
	glm::vec3 aLoLargo = glm::vec3(cos(giro * toRadians), 0.0f, -sin(giro * toRadians));
	float avance = 0.0f;
	int i = primera;
	while (avance < largo - 0.5f)
	{
		float ancho = anchosCasa[i % numModelosCasa];
		float alto = altosCasa[i % numModelosCasa];
		glm::vec3 colorCuerpo = coloresCasa[i % numColoresCasa];
		glm::vec3 colorFachada = colorCuerpo;
		if (estilo == estiloCantera)
		{
			alto += 1.5f;
			colorCuerpo = glm::vec3(0.70f, 0.68f, 0.54f);
			colorFachada = glm::vec3(1.0f, 1.0f, 1.0f);
		}
		else if (estilo == estiloPortal)
		{
			ancho = 10.0f;
			alto = 9.0f;
			colorCuerpo = glm::vec3(0.90f, 0.80f, 0.60f);
			colorFachada = glm::vec3(1.0f, 1.0f, 1.0f);
		}
		//la ultima casa se recorta para no pasarse del largo de la fila
		if (avance + ancho > largo)
		{
			ancho = largo - avance;
		}
		glm::vec3 centroFrente = inicio + aLoLargo * (avance + ancho / 2.0f);

		//cuerpo de la casa, que va del frente hacia atras
		glm::mat4 model(1.0);
		model = glm::translate(model, centroFrente - haciaLaCalle * (fondo / 2.0f) + glm::vec3(0.0f, alto / 2.0f, 0.0f));
		model = glm::rotate(model, giro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(ancho, alto, fondo));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCuerpo));
		plainTexture.UseTexture();
		meshListModel[2]->RenderMeshModel();

		//tinaco negro en la azotea de una de cada tres casas
		if (i % 3 == 0)
		{
			DibujarBloque(centroFrente - haciaLaCalle * (fondo * 0.6f) + glm::vec3(0.0f, alto, 0.0f),
				glm::vec3(1.0f, 1.3f, 1.0f), glm::vec3(0.12f, 0.12f, 0.13f), uniformModel, uniformColor);
		}

		//fachada, las casas altas son de dos pisos
		if (estilo == estiloCantera)
		{
			fachadaCanteraTexture.UseTexture();
		}
		else if (estilo == estiloPortal)
		{
			fachadaPortalTexture.UseTexture();
		}
		else if (alto >= 7.0f)
		{
			fachadaCasa2Texture.UseTexture();
		}
		else
		{
			fachadaCasa1Texture.UseTexture();
		}
		DibujarPlano(centroFrente + haciaLaCalle * 0.02f, giro, ancho, alto, colorFachada, uniformModel, uniformColor);

		//la primera y la ultima casa de la fila quedan en esquina, asi que tambien llevan fachada en el costado que da a la otra calle
		//en los portales no se pone porque sus costados son muy largos y los arcos se verian estirados
		glm::vec3 centroCostado = centroFrente - haciaLaCalle * (fondo / 2.0f);
		if (avance == 0.0f && estilo != estiloPortal)
		{
			DibujarPlano(centroCostado - aLoLargo * (ancho / 2.0f + 0.02f), giro - 90.0f, fondo, alto, colorFachada, uniformModel, uniformColor);
		}
		if (avance + ancho >= largo - 0.5f && estilo != estiloPortal)
		{
			DibujarPlano(centroCostado + aLoLargo * (ancho / 2.0f + 0.02f), giro + 90.0f, fondo, alto, colorFachada, uniformModel, uniformColor);
		}

		avance += ancho;
		i++;
	}
}

//todas las filas de casas de la escena
void DibujarCasas(GLuint uniformModel, GLuint uniformColor)
{
	//poniente del andador, las fachadas ven hacia X positivo
	//la primera fila empieza junto al costado norte de la Sangre de Cristo
	DibujarFilaCasas(glm::vec3(-6.0f, 0.0f, -101.0f), 90.0f, 77.0f, 12.0f, estiloColores, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-6.0f, 0.0f, -26.0f), 90.0f, 48.0f, 12.0f, estiloColores, 2, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-6.0f, 0.0f, 30.0f), 90.0f, 44.0f, 12.0f, estiloCantera, 1, uniformModel, uniformColor);
	//oriente del andador, las fachadas ven hacia X negativo
	DibujarFilaCasas(glm::vec3(6.0f, 0.0f, -106.0f), -90.0f, 20.0f, 12.0f, estiloColores, 4, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(6.0f, 0.0f, -44.0f), -90.0f, 18.0f, 9.0f, estiloColores, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(6.0f, 0.0f, -14.0f), -90.0f, 44.0f, 12.0f, estiloCantera, 2, uniformModel, uniformColor);

	//calles que cruzan, del lado poniente
	DibujarFilaCasas(glm::vec3(-18.0f, 0.0f, -178.0f), 180.0f, 36.0f, 12.0f, estiloColores, 5, uniformModel, uniformColor);
	//esta llega hasta la parte de atras de la Sangre de Cristo, en x = -27
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, -86.0f), 0.0f, 27.0f, 12.0f, estiloColores, 3, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-18.0f, 0.0f, -74.0f), 180.0f, 36.0f, 12.0f, estiloColores, 4, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, -26.0f), 0.0f, 36.0f, 12.0f, estiloColores, 6, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-18.0f, 0.0f, -14.0f), 180.0f, 36.0f, 12.0f, estiloColores, 3, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, 30.0f), 0.0f, 36.0f, 12.0f, estiloCantera, 0, uniformModel, uniformColor);

	//calles que cruzan, del lado oriente, y las casas que ven al atrio y al jardin Labastida
	//la manzana entre el atrio y Manuel Bravo es angosta, por eso las casas que ven al atrio y las de Manuel Bravo tienen menos fondo
	DibujarFilaCasas(glm::vec3(56.0f, 0.0f, -106.0f), 180.0f, 38.0f, 9.0f, estiloColores, 1, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(18.0f, 0.0f, -86.0f), 0.0f, 50.0f, 11.0f, estiloColores, 6, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(42.0f, 0.0f, -74.0f), -90.0f, 28.0f, 12.0f, estiloColores, 2, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(68.0f, 0.0f, -74.0f), 180.0f, 14.0f, 12.0f, estiloColores, 5, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(68.0f, 0.0f, -44.0f), 180.0f, 50.0f, 9.0f, estiloColores, 3, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(18.0f, 0.0f, -26.0f), 0.0f, 50.0f, 9.0f, estiloColores, 1, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(68.0f, 0.0f, -14.0f), 180.0f, 50.0f, 12.0f, estiloColores, 6, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(18.0f, 0.0f, 30.0f), 0.0f, 50.0f, 12.0f, estiloCantera, 4, uniformModel, uniformColor);

	//fila al norte de Gurrion, cierra la vista al norte
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, -190.0f), 0.0f, 162.0f, 12.0f, estiloColores, 0, uniformModel, uniformColor);

	//portales del Zocalo en el poniente, el oriente y el sur
	DibujarFilaCasas(glm::vec3(-35.0f, 0.0f, 112.0f), 90.0f, 70.0f, 20.0f, estiloPortal, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(35.0f, 0.0f, 42.0f), -90.0f, 70.0f, 35.0f, estiloPortal, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(70.0f, 0.0f, 112.0f), 180.0f, 126.0f, 16.0f, estiloPortal, 0, uniformModel, uniformColor);

	//relleno del centro de cada manzana, mas bajo que las casas, para que desde arriba no se vea hueco
	//queda 6 m adentro de la orilla, asi sus lados quedan escondidos dentro de las casas
	//las dos primeras manzanas no llevan relleno porque una es la fila del norte y la otra es Santo Domingo
	plainTexture.UseTexture();
	for (int i = 2; i < numManzanas; i++)
	{
		const Rectangulo& m = manzanas[i];
		glm::vec3 centro = glm::vec3((m.x0 + m.x1) / 2.0f, 0.0f, (m.z0 + m.z1) / 2.0f);
		glm::vec3 tamano = glm::vec3(m.x1 - m.x0 - 12.0f, 4.5f, m.z1 - m.z0 - 12.0f);
		DibujarBloque(centro, tamano, glm::vec3(0.74f, 0.70f, 0.64f), uniformModel, uniformColor);
	}
}


// ================== 4. PERSONAJE JERARQUICO ==================
//cada parte se dibuja a partir de la matriz de su padre, del cuerpo salen la cabeza, los brazos y las piernas
//y del brazo sale el antebrazo, asi al girar el hombro tambien se mueve el antebrazo

//angulos de las articulaciones, en grados
//el hombro gira hacia adelante sobre X, donde un angulo negativo sube el brazo al frente,
//y hacia el lado sobre Z, donde uno positivo mueve la mano hacia X positivo
//codos, caderas y rodillas giran sobre X, con un angulo negativo el codo dobla hacia el frente
//y con uno positivo la rodilla dobla hacia atras
//la cabeza gira sobre Y para voltear
struct PosePersona
{
	float hombroIzqAdelante, hombroIzqLado, codoIzq;
	float hombroDerAdelante, hombroDerLado, codoDer;
	float caderaIzq, rodillaIzq;
	float caderaDer, rodillaDer;
	float cabeza;
};

//pose del muneco, empieza parado con todos los angulos en 0
//los valores van en el mismo orden que en PosePersona, primero el brazo izquierdo, luego el derecho, despues las piernas y al final la cabeza
PosePersona poseMuneco = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

//medidas del esqueleto en metros
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

//dibuja un brazo o una pierna de dos partes, la primera gira en la articulacion
//y la segunda cuelga de ella y se dobla en el codo o la rodilla
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

//teclas de prueba que giran las articulaciones del muneco
//aqui se escribe la pose y el dibujo en el while solo la lee
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
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();
	CreateObjects();
	CrearCubo();
	CrearEscenario();
	CrearPlano();
	CrearJardineras();
	CreateShaders();

	//Camera recibe la posicion inicial, el vector up, yaw, pitch, la velocidad de movimiento y la velocidad de giro
	//con yaw de 90 la camara mira hacia Z positivo, con 0 hacia X positivo y con 180 hacia X negativo
	//la vista inicial esta sobre el andador frente al atrio, a 10 m de altura, mirando hacia el sur
	camera = Camera(glm::vec3(0.0f, 10.0f, -150.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -15.0f, 0.3f, 0.5f);
	//la de la fachada esta en el atrio y mira hacia el oriente, donde esta Santo Domingo, un poco hacia arriba para ver las torres
	//velocidades en 0 porque esta camara no se mueve
	camaraFachada = Camera(glm::vec3(12.0f, 9.0f, -144.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 12.0f, 0.0f, 0.0f);
	//la del desfile esta en la esquina del atrio junto al andador y mira hacia el sur, un poco cargada hacia la Sangre de Cristo
	camaraDesfile = Camera(glm::vec3(5.0f, 2.6f, -112.0f), glm::vec3(0.0f, 1.0f, 0.0f), 110.0f, -8.0f, 0.0f, 0.0f);
	//camara con la que se dibuja la escena, cambia en el while segun la tecla C
	Camera* camaraActual = &camera;

	// ================== 5. CARGA DE RECURSOS ==================

	//en las texturas primero se asigna la ruta y luego se carga
	//LoadTextureA es para imagenes con canal alfa como png o tga, y LoadTexture para las que no tienen, como jpg
	andadorTexture = Texture("Textures/cantera_losas.jpg");
	andadorTexture.LoadTexture();
	empedradoTexture = Texture("Textures/empedrado.jpg");
	empedradoTexture.LoadTexture();
	banquetaTexture = Texture("Textures/banqueta.jpg");
	banquetaTexture.LoadTexture();
	plazaTexture = Texture("Textures/plaza_losas.jpg");
	plazaTexture.LoadTexture();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	fachadaCasa1Texture = Texture("Textures/fachada_casa1.jpg");
	fachadaCasa1Texture.LoadTexture();
	fachadaCasa2Texture = Texture("Textures/fachada_casa2.jpg");
	fachadaCasa2Texture.LoadTexture();
	fachadaCanteraTexture = Texture("Textures/fachada_cantera.jpg");
	fachadaCanteraTexture.LoadTexture();
	fachadaPortalTexture = Texture("Textures/fachada_portal.jpg");
	fachadaPortalTexture.LoadTexture();
	telonTexture = Texture("Textures/telon.png");
	telonTexture.LoadTextureA();
	canteraTexture = Texture("Textures/cantera_muro.jpg");
	canteraTexture.LoadTexture();
	tierraTexture = Texture("Textures/tierra_roja.jpg");
	tierraTexture.LoadTexture();

	//los modelos traen su propia textura, que se busca en la carpeta Textures
	farolaBrazo = Model();
	farolaBrazo.LoadModel("Models/farola_brazo.obj");
	farolaGlobos = Model();
	farolaGlobos.LoadModel("Models/farola_globos.obj");
	santoDomingo = Model();
	santoDomingo.LoadModel("Models/santo_domingo.obj");
	sangreDeCristo = Model();
	sangreDeCristo.LoadModel("Models/sangre_de_cristo.obj");
	kiosco = Model();
	kiosco.LoadModel("Models/kiosco.obj");
	fuenteZocalo = Model();
	fuenteZocalo.LoadModel("Models/fuente_zocalo.obj");

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


	// ---------- luces ----------
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

	//farolas en las banquetas del andador, una de cada lado cada 16 m mas o menos, y cuatro en el Zocalo alrededor del kiosco
	//estan sobre la banqueta, por eso su base esta a la altura de la banqueta
	//las primeras 12 son de brazo y las ultimas 4, las del Zocalo, son de globos
	const int numFarolas = 16; //no debe pasar de MAX_POINT_LIGHTS
	const int numFarolasAndador = 12;
	glm::vec3 posicionFarolas[numFarolas] = {
		glm::vec3(-5.0f, altoBanqueta, -176.0f), glm::vec3(5.0f, altoBanqueta, -160.0f), glm::vec3(-5.0f, altoBanqueta, -144.0f),
		glm::vec3(5.0f, altoBanqueta, -128.0f), glm::vec3(-5.0f, altoBanqueta, -112.0f), glm::vec3(5.0f, altoBanqueta, -96.0f),
		glm::vec3(-5.0f, altoBanqueta, -66.0f), glm::vec3(5.0f, altoBanqueta, -50.0f), glm::vec3(-5.0f, altoBanqueta, -34.0f),
		glm::vec3(5.0f, altoBanqueta, -6.0f), glm::vec3(-5.0f, altoBanqueta, 10.0f), glm::vec3(5.0f, altoBanqueta, 26.0f),
		glm::vec3(-10.0f, altoBanqueta, 62.0f), glm::vec3(10.0f, altoBanqueta, 62.0f),
		glm::vec3(-10.0f, altoBanqueta, 92.0f), glm::vec3(10.0f, altoBanqueta, 92.0f)
	};
	//el farol de la farola de brazo cuelga 22 cm hacia X negativo de su centro, a 3.85 m de alto
	//las de la banqueta poniente se giran 180 grados para que el farol quede del lado de la calle
	float giroFarolas[numFarolas];
	glm::vec3 posicionFaroles[numFarolas];
	for (int i = 0; i < numFarolas; i++)
	{
		if (i >= numFarolasAndador)
		{
			//en la de globos la luz va entre los dos globos
			giroFarolas[i] = 0.0f;
			posicionFaroles[i] = posicionFarolas[i] + glm::vec3(0.0f, 3.9f, 0.0f);
		}
		else if (posicionFarolas[i].x < 0.0f)
		{
			giroFarolas[i] = 180.0f;
			posicionFaroles[i] = posicionFarolas[i] + glm::vec3(0.22f, 3.85f, 0.0f);
		}
		else
		{
			giroFarolas[i] = 0.0f;
			posicionFaroles[i] = posicionFarolas[i] + glm::vec3(-0.22f, 3.85f, 0.0f);
		}
	}
	//cada farola tiene una luz puntual amarilla en su farol
	//la atenuacion hace que alumbre un circulo de unos 6 m de radio alrededor del poste
	for (int i = 0; i < numFarolas; i++)
	{
		pointLights[i] = PointLight(1.0f, 0.8f, 0.5f,
			0.0f, 1.2f,
			posicionFaroles[i].x, posicionFaroles[i].y, posicionFaroles[i].z,
			1.0f, 0.09f, 0.032f);
	}
	//contador de luces puntuales, en el while cambia a 0 de tarde y a numFarolas de noche
	unsigned int pointLightCount = 0;

	//contador de spotlights, no puede pasar de MAX_SPOT_LIGHTS
	unsigned int spotLightCount = 0;
	//luz de la fachada de Santo Domingo, sale del piso del atrio y apunta hacia arriba y hacia la fachada
	spotLights[0] = SpotLight(1.0f, 0.9f, 0.75f,
		0.0f, 1.2f,
		36.0f, 0.5f, -142.0f,
		1.0f, 0.6f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		45.0f);
	spotLightCount++;

	//luz del kiosco, esta debajo del techo y apunta hacia abajo para alumbrar la plataforma donde toca la banda
	spotLights[1] = SpotLight(1.0f, 0.85f, 0.6f,
		0.0f, 1.5f,
		0.0f, 7.0f, 77.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		55.0f);
	spotLightCount++;

	//luz de la fachada de la Sangre de Cristo, sale del piso del andador frente a la puerta y apunta hacia el poniente y hacia arriba
	spotLights[2] = SpotLight(1.0f, 0.88f, 0.7f,
		0.0f, 1.6f,
		3.0f, 0.3f, -93.6f,
		-1.0f, 0.5f, 0.0f,
		1.0f, 0.045f, 0.0075f,
		40.0f);
	spotLightCount++;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;

	//telones al final de cada calle que cruza, con la imagen de la calle que sigue
	//de cada uno se guarda el centro de su orilla de abajo y hacia donde ve
	const int numTelones = 8;
	glm::vec3 posicionTelones[numTelones] = {
		glm::vec3(-56.0f, 0.0f, -184.0f), glm::vec3(110.0f, 0.0f, -184.0f),	//Gurrion
		glm::vec3(-56.0f, 0.0f, -80.0f), glm::vec3(70.0f, 0.0f, -80.0f),		//Abasolo y Manuel Bravo
		glm::vec3(-56.0f, 0.0f, -20.0f), glm::vec3(70.0f, 0.0f, -20.0f),		//Morelos
		glm::vec3(-56.0f, 0.0f, 37.0f), glm::vec3(70.0f, 0.0f, 37.0f)			//Independencia
	};
	//los de la orilla poniente ven hacia X positivo y los de la oriente hacia X negativo
	float giroTelones[numTelones] = { 90.0f, -90.0f, 90.0f, -90.0f, 90.0f, -90.0f, 90.0f, -90.0f };

	//el angulo de glm::perspective va en radianes, por eso 45.0f da una vista de unos 58 grados y no de 45
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// ================== 6. CICLO PRINCIPAL ==================
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
			//la camara libre no puede salir de la ciudad ni subir mas de 30 m, asi no se ve donde se acaba la escena
			camera.limitarPosicion(glm::vec3(-54.0f, 1.0f, -200.0f), glm::vec3(68.0f, 30.0f, 126.0f));
		}

		//las teclas de prueba escriben la pose del muneco y el dibujo de abajo solo la lee
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

		glm::mat4 model(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //con blanco la textura se ve con sus colores normales

		// ---------- objetos opacos ----------
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//los pisos ya tienen sus coordenadas puestas en el mundo, asi que van con la matriz identidad
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//el piso lejano usa la textura blanca con un color parecido al de los techos
		plainTexture.UseTexture();
		glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.55f, 0.50f, 0.45f)));
		meshListModel[0]->RenderMeshModel();

		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		empedradoTexture.UseTexture();
		meshListModel[3]->RenderMeshModel();
		andadorTexture.UseTexture();
		meshListModel[4]->RenderMeshModel();
		banquetaTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();
		plazaTexture.UseTexture();
		meshListModel[6]->RenderMeshModel();

		DibujarCasas(uniformModel, uniformColor);

		//Santo Domingo en el oriente del atrio, con la puerta en x = 56 y la fachada hacia el poniente
		//el modelo tiene la fachada hacia Z positivo, por eso se gira -90 grados para que vea hacia X negativo
		//asi el ex convento, que en el modelo esta hacia X negativo, queda al norte
		//los barandales y las rejas son png con transparencia, el discard del shader quita lo que no se ve aunque se dibujen aqui
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(56.0f, altoBanqueta, -139.5f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		santoDomingo.RenderModel();

		//Sangre de Cristo en la esquina noroeste de Abasolo, con la fachada hacia el oriente, que da al andador
		//el modelo tiene la fachada hacia Z positivo, por eso se gira 90 grados para que vea hacia X positivo
		//la fachada queda en x = -9 y la orilla de los escalones en x = -6, junto a la banqueta, y su costado sur da a Abasolo
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-9.0f, altoBanqueta, -93.6f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		sangreDeCristo.RenderModel();

		//kiosco al centro del Zocalo, sus escaleras ven hacia los cuatro lados
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, altoBanqueta, 77.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		kiosco.RenderModel();

		//jardineras del Zocalo, igual que los pisos ya tienen sus coordenadas en el mundo
		model = glm::mat4(1.0);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		canteraTexture.UseTexture();
		meshListModel[8]->RenderMeshModel();
		tierraTexture.UseTexture();
		meshListModel[9]->RenderMeshModel();

		//farolas, el mismo modelo se dibuja en cada lugar con su giro
		//los modelos usan su propia textura, solo se les pone el color blanco para que se vea tal cual
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		for (int i = 0; i < numFarolas; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posicionFarolas[i]);
			model = glm::rotate(model, giroFarolas[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i < numFarolasAndador)
			{
				farolaBrazo.RenderModel();
			}
			else
			{
				farolaGlobos.RenderModel();
			}
		}

		//muneco de cubos sobre el andador, frente a la Sangre de Cristo, mirando hacia el sur
		//se sube 1 cm igual que el andador para que los pies queden encima de el
		plainTexture.UseTexture();
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.01f, -100.0f));
		DibujarMuneco(model, poseMuneco, uniformModel, uniformColor);

		// ---------- objetos con transparencia ----------
		//van al final, despues de todos los opacos, para que se vea lo que esta detras de las partes transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//telones al final de las calles que cruzan, con la imagen de la calle que sigue
		telonTexture.UseTexture();
		for (int i = 0; i < numTelones; i++)
		{
			DibujarPlano(posicionTelones[i], giroTelones[i], 12.0f, 12.0f, color, uniformModel, uniformColor);
		}

		//fuente al sur del kiosco, va aqui porque su agua es un poco transparente y deja ver el fondo de la pila
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, altoBanqueta, 100.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		fuenteZocalo.RenderModel();

		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
