//solo aqui, si no da error de simbolos repetidos
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
#include "Sonido.h"

const float toRadians = 3.14159265f / 180.0f;

// ================== 1. VARIABLES GLOBALES ==================

Window mainWindow;
std::vector<MeshModel*> meshListModel; //mallas hechas a mano
std::vector<Shader> shaderList;

//la libre es camera, las demas son fijas
Camera camera;
Camera camaraFachada;
Camera camaraDesfile;
Camera camaraZocalo;
Camera camaraAerea;

//texturas en Textures/
Texture andadorTexture;
Texture empedradoTexture;
Texture banquetaTexture;
Texture plazaTexture;
Texture plainTexture;		//blanca, el objeto se ve con el color que se le pase
//aplanados claros, para pintarlos con el color de cada casa
Texture fachadaCasa1Texture;
Texture fachadaCasa2Texture;
Texture fachadaCanteraTexture;	//cantera, no se pinta
Texture fachadaPortalTexture;	//ya viene pintada
//jardineras del atrio y del Zocalo
Texture canteraTexture;			//la imagen cubre 2 m
Texture tierraTexture;			//la imagen cubre 2 m
//canal alfa, se dibuja al final con transparencia
Texture telonTexture;
Texture papelPicadoTexture;
Texture maderaTexture;		//tablones, para las estanterias de macetas

//archivos en Models/, texturas en Textures/
Model farolaBrazo;
Model farolaGlobos;
Model santoDomingo;		//fachada hacia Z positivo
Model sangreDeCristo;	//fachada hacia Z positivo
Model kiosco;			//centrado en el origen
Model fuenteZocalo;		//centrada en el origen
Model agave;			//1.5 m de ancho
Model flamboyan;		//7 m, hojas en planos con transparencia
Model palmera;			//8.7 m, pencas en planos con transparencia
//de 7 y 5.5 m, se van turnando
Model arbolZocalo1;
Model arbolZocalo2;
//los ligeros de 8 y 7.5 m, la jacaranda de 9 m
Model arbolLigero1;
Model arbolLigero2;
Model jacaranda;
Model cacaloxochitl;	//4 m
//se van turnando en las estanterias de las fachadas
const int numModelosMaceta = 6;
Model macetas[numModelosMaceta];
//flores sin maceta para las jardineras, miden de 36 a 45 cm
const int numModelosFlor = 6;
Model flores[numModelosFlor];
Model banca;			//1.8 m, respaldo hacia Z negativo
Model carritoElotes;	//letrero hacia Z positivo
Model bugambilia;		//3.7 m
Model bugambiliaMuro;	//el muro va en Z cero
//puestos y lo que se vende en ellos
Model mesaToldo;
Model tendedero;		//de frente hacia Z positivo
Model canasta;
Model petate;			//1.2 por 1.6 m
Model cazuela;
Model olla;
Model taza;
Model panYema;
Model alebrijeCoyote;
Model alebrijePajaro;
Model alebrijePerrito;
//piezas de los personajes del desfile, cada una con su origen en la articulacion donde gira
Model hombreTorso;
Model hombreCabeza;
Model hombreBrazoIzq;
Model hombreAntebrazoIzq;
Model hombreBrazoDer;
Model hombreAntebrazoDer;
Model hombreMusloIzq;
Model hombrePiernaIzq;
Model hombreMusloDer;
Model hombrePiernaDer;
Model chinaTorso;		//cabeza y blusa, los brazos son los del hombre
Model chinaFalda;
Model chinaCanasta;
Model dulce;
Model marmota;			//globo de manta de 1.5 m, origen donde entra el palo
Model marmotaPalo;		//1.8 m
Model torito;
Model toritoRueda;
Model toritoChispa;
Model trompeta;			//la boquilla en el origen, apunta hacia Z positivo
Model globo;			//blanco para pintarlo de color, origen en la punta del hilo
//espectadores: el hombre usa las piezas del desfile con otra ropa y la mujer el torso de la china
Model transeunteTorso;	//el torso del hombre sin paliacate
Model mujerFalda;		//blanca para pintarla de color

//ropa de cada variante; la falda de la primera china y la primera blusa son las de su modelo
const int numRopas = 4;
Texture faldaChinaTexture[numRopas];
Texture ropaHombreTexture[numRopas];
Texture ropaMujerTexture[numRopas];		//mangas, pantorrillas y zapatos, en la textura del hombre
Texture blusaMujerTexture[numRopas];

//uno para la tarde y otro para la noche, segun la tecla N
Skybox skyboxTarde;
Skybox skyboxNoche;

//materiales
Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

//sol de tarde y sol de noche, se manda uno segun la tecla N
DirectionalLight luzTarde;
DirectionalLight luzNoche;
//maximo en CommonValues.h
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

//el skybox carga los suyos
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";


// ================== 2. GEOMETRIA HECHA A MANO ==================

//rectangulo acostado de (x0, z0) a (x1, z1) a la altura y, normal hacia arriba
//u y v son x y z entre los metros de la imagen, asi los pedazos vecinos coinciden
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

//rectangulo parado de (xa, za) a (xb, zb), de y0 a y1
//nx y nz apuntan hacia afuera del objeto
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

//bloque bajo con tapa y cuatro lados, para banquetas y plazas
void AgregarPlataforma(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices,
	float x0, float z0, float x1, float z1, float alto, float metrosImagen)
{
	AgregarRectanguloPiso(vertices, indices, x0, z0, x1, z1, alto, metrosImagen);
	AgregarRectanguloPared(vertices, indices, x1, z0, x0, z0, 0.0f, alto, 0.0f, -1.0f, metrosImagen);	//lado norte
	AgregarRectanguloPared(vertices, indices, x0, z1, x1, z1, 0.0f, alto, 0.0f, 1.0f, metrosImagen);	//lado sur
	AgregarRectanguloPared(vertices, indices, x0, z0, x0, z1, 0.0f, alto, -1.0f, 0.0f, metrosImagen);	//lado poniente
	AgregarRectanguloPared(vertices, indices, x1, z1, x1, z0, 0.0f, alto, 1.0f, 0.0f, metrosImagen);	//lado oriente
}

//copia los vertices a la GPU y la mete a meshListModel
void GuardarMalla(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices)
{
	MeshModel* malla = new MeshModel();
	malla->CreateMeshModel(vertices.data(), indices.data(), (unsigned int)vertices.size(), (unsigned int)indices.size());
	meshListModel.push_back(malla);
}

//las mallas se dibujan por su indice en meshListModel
void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	//piso lejano de 1000 por 1000 m, llena lo que queda fuera de la ciudad
	GLfloat floorVertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-470.0f, -0.05f, -540.0f,	0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		530.0f, -0.05f, -540.0f,	1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		-470.0f, -0.05f, 460.0f,	0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		530.0f, -0.05f, 460.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f
	};

	//dos planos cruzados para plantas
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4, 5, 6,
	   4, 6, 7
	};

	//normales como el piso
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


//cubo de lado 1 centrado en el origen
void CrearCubo()
{
	//normales hacia afuera, como en Blender
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

		// back, la S va al reves para que no salga en espejo
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


//plano de 1 por 1, base en el origen, cara hacia Z positivo
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
//1 unidad es 1 metro, norte hacia Z negativo, sur hacia Z positivo
//andador de norte a sur sobre x cero, calle de 8 m (de -4 a 4), banquetas de 2 m

//rectangulo en el piso, de la esquina noroeste a la sureste
struct Rectangulo
{
	float x0, z0, x1, z1;
};

const float altoBanqueta = 0.15f;

//banqueta de 15 cm, casas 2 m adentro de la orilla
const int numManzanas = 12;
const Rectangulo manzanas[numManzanas] = {
	{ -56.0f, -204.0f, 110.0f, -188.0f },	//al norte de Gurrion
	{ 56.0f, -180.0f, 110.0f, -108.0f },	//Santo Domingo
	{ -56.0f, -180.0f, -4.0f, -84.0f },		//poniente del atrio, con la Sangre de Cristo
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

//tambien 15 cm arriba de la calle
const int numPlazas = 3;
const Rectangulo plazas[numPlazas] = {
	{ 4.0f, -180.0f, 56.0f, -108.0f },		//atrio de Santo Domingo
	{ 4.0f, -76.0f, 40.0f, -46.0f },		//jardin Labastida
	{ -35.0f, 42.0f, 35.0f, 112.0f }		//Zocalo
};

//en los cruces se ve el empedrado de la calle
const int numTramos = 3;
const Rectangulo tramosAndador[numTramos] = {
	{ -4.0f, -180.0f, 4.0f, -84.0f },		//de Gurrion a Abasolo
	{ -4.0f, -76.0f, 4.0f, -24.0f },		//de Abasolo a Morelos
	{ -4.0f, -16.0f, 4.0f, 32.0f }			//de Morelos a Independencia
};

//las dos primeras son del atrio, con agaves, a los lados del paso a la puerta
//las cuatro que siguen son del Zocalo, alrededor del kiosco, dejando libres los pasillos
//la de los flamboyanes va junto a Gurrion y los macetones de las palmeras al final
const int numJardineras = 10;
const int numJardinerasAtrio = 2;
const Rectangulo jardineras[numJardineras] = {
	{ 20.0f, -166.5f, 46.0f, -146.5f }, { 20.0f, -132.5f, 46.0f, -112.5f },
	{ -29.0f, 49.0f, -11.0f, 63.0f }, { 11.0f, 49.0f, 29.0f, 63.0f },
	{ -29.0f, 91.0f, -11.0f, 105.0f }, { 11.0f, 91.0f, 29.0f, 105.0f },
	{ 7.0f, -178.5f, 49.0f, -175.0f },
	{ 8.8f, -111.2f, 11.2f, -108.8f }, { 24.8f, -111.2f, 27.2f, -108.8f }, { 40.8f, -111.2f, 43.2f, -108.8f }
};
const float altoJardinera = 0.6f;	//lo que sobresalen del piso de la plaza
const float orillaJardinera = 0.5f;	//ancho del borde de cantera que rodea la tierra

//mallas de calles, andador, banquetas y plazas
void CrearEscenario()
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	//empedrado de 2 m, el segundo rectangulo queda detras de Santo Domingo
	AgregarRectanguloPiso(vertices, indices, -58.0f, -206.0f, 72.0f, 130.0f, 0.0f, 2.0f);
	AgregarRectanguloPiso(vertices, indices, 72.0f, -206.0f, 112.0f, -106.0f, 0.0f, 2.0f);
	GuardarMalla(vertices, indices); //meshListModel[3]

	//1 cm arriba del empedrado para que no parpadee
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

	//la imagen cubre 4 m, las losas son mas grandes
	vertices.clear();
	indices.clear();
	for (int i = 0; i < numPlazas; i++)
	{
		AgregarPlataforma(vertices, indices, plazas[i].x0, plazas[i].z0, plazas[i].x1, plazas[i].z1, altoBanqueta, 4.0f);
	}
	GuardarMalla(vertices, indices); //meshListModel[6]
}

//una malla con la cantera y otra con la tierra
//la tierra va 1 cm arriba para que no parpadee
//meshListModel[8] y [9]
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

//tira de papel picado de x -6 a 6, cara hacia Z positivo
//el cordel baja en medio como una parabola, por eso se arma en tramos
//cada banderita mide 50 cm, la imagen de ocho se repite tres veces
const float anchoTira = 12.0f;
const float caidaTira = 0.6f;		//cuanto baja el cordel en medio
const float altoTira = 0.43f;		//alto de las banderitas, con la orilla transparente arriba
const int tramosTira = 12;
void CrearTiraPapelPicado()
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	for (int i = 0; i <= tramosTira; i++)
	{
		float x = -anchoTira / 2.0f + anchoTira * i / tramosTira;
		//la caida es 0 en las orillas y caidaTira en medio
		float mitad = anchoTira / 2.0f;
		float yCordel = -caidaTira * (1.0f - (x / mitad) * (x / mitad));
		float u = 3.0f * i / tramosTira;
		vertices.insert(vertices.end(), {
			x, yCordel - altoTira, 0.0f,	u, 0.0f,	0.0f, 0.0f, 1.0f,
			x, yCordel, 0.0f,				u, 1.0f,	0.0f, 0.0f, 1.0f });
	}
	//une cada corte con el siguiente
	for (int i = 0; i < tramosTira; i++)
	{
		unsigned int abajo = i * 2;
		unsigned int arriba = i * 2 + 1;
		unsigned int abajoSig = i * 2 + 2;
		unsigned int arribaSig = i * 2 + 3;
		indices.insert(indices.end(), { abajo, abajoSig, arribaSig, arribaSig, arriba, abajo });
	}
	GuardarMalla(vertices, indices); //meshListModel[10]
}


//dibuja el cubo como bloque de color, posicion es el centro de la base
//se sube la mitad del alto para apoyarlo en el piso
void DibujarBloque(glm::vec3 posicion, glm::vec3 tamano, glm::vec3 color, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model(1.0);
	model = glm::translate(model, posicion + glm::vec3(0.0f, tamano.y / 2.0f, 0.0f));
	model = glm::scale(model, tamano);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshListModel[2]->RenderMeshModel();
}

//plano parado, posicion es el centro de su orilla de abajo
//giro 0 ve hacia Z positivo, 90 hacia X positivo, 180 hacia Z negativo
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

//modelo de Assimp con el centro de su base en posicion
void DibujarModelo(Model& modelo, glm::vec3 posicion, float giro, float escala, GLuint uniformModel)
{
	glm::mat4 model(1.0);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala, escala, escala));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	modelo.RenderModel();
}


//cada casa es un cubo de color, la fachada va en un plano 2 cm enfrente
//medidas y colores se van turnando
const int numModelosCasa = 7;
const float anchosCasa[numModelosCasa] = { 8.0f, 6.5f, 10.0f, 7.0f, 9.0f, 6.0f, 11.0f };
const float altosCasa[numModelosCasa] = { 6.0f, 7.5f, 5.5f, 8.0f, 6.5f, 7.2f, 5.8f };
const int numColoresCasa = 8;
//colores de fachadas del centro de Oaxaca
const glm::vec3 coloresCasa[numColoresCasa] = {
	glm::vec3(0.95f, 0.78f, 0.35f), glm::vec3(0.80f, 0.32f, 0.28f), glm::vec3(0.38f, 0.56f, 0.88f),
	glm::vec3(0.93f, 0.64f, 0.68f), glm::vec3(0.96f, 0.94f, 0.88f), glm::vec3(0.88f, 0.58f, 0.30f),
	glm::vec3(0.58f, 0.76f, 0.56f), glm::vec3(0.92f, 0.56f, 0.46f)
};

//estilos de las filas
const int estiloColores = 0;	//pintadas de colores
const int estiloCantera = 1;	//edificios de cantera de dos pisos
const int estiloPortal = 2;		//todos del mismo ancho y alto

//inicio es la esquina donde empieza la fila
//la fila avanza a la derecha de quien ve las fachadas, largo y fondo en metros
//primera elige con que medida y color empieza
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
		//la ultima casa se recorta al largo de la fila
		if (avance + ancho > largo)
		{
			ancho = largo - avance;
		}
		glm::vec3 centroFrente = inicio + aLoLargo * (avance + ancho / 2.0f);

		//cuerpo de la casa, del frente hacia atras
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

		//las casas altas son de dos pisos
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

		//la primera y la ultima casa llevan fachada en el costado de la esquina
		//en los portales no, los arcos se verian estirados
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
	//la primera fila empieza junto a la Sangre de Cristo
	DibujarFilaCasas(glm::vec3(-6.0f, 0.0f, -101.0f), 90.0f, 77.0f, 12.0f, estiloColores, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-6.0f, 0.0f, -26.0f), 90.0f, 48.0f, 12.0f, estiloColores, 2, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-6.0f, 0.0f, 30.0f), 90.0f, 44.0f, 12.0f, estiloCantera, 1, uniformModel, uniformColor);
	//oriente del andador, las fachadas ven hacia X negativo
	DibujarFilaCasas(glm::vec3(6.0f, 0.0f, -106.0f), -90.0f, 20.0f, 12.0f, estiloColores, 4, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(6.0f, 0.0f, -44.0f), -90.0f, 18.0f, 9.0f, estiloColores, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(6.0f, 0.0f, -14.0f), -90.0f, 44.0f, 12.0f, estiloCantera, 2, uniformModel, uniformColor);

	//calles que cruzan, del lado poniente
	DibujarFilaCasas(glm::vec3(-18.0f, 0.0f, -178.0f), 180.0f, 36.0f, 12.0f, estiloColores, 5, uniformModel, uniformColor);
	//llega hasta la parte de atras de la Sangre de Cristo, en x -27
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, -86.0f), 0.0f, 27.0f, 12.0f, estiloColores, 3, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-18.0f, 0.0f, -74.0f), 180.0f, 36.0f, 12.0f, estiloColores, 4, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, -26.0f), 0.0f, 36.0f, 12.0f, estiloColores, 6, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-18.0f, 0.0f, -14.0f), 180.0f, 36.0f, 12.0f, estiloColores, 3, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, 30.0f), 0.0f, 36.0f, 12.0f, estiloCantera, 0, uniformModel, uniformColor);

	//calles que cruzan del lado oriente, y las que ven al atrio y al jardin Labastida
	//la manzana entre el atrio y Manuel Bravo es angosta, por eso tienen menos fondo
	DibujarFilaCasas(glm::vec3(56.0f, 0.0f, -106.0f), 180.0f, 38.0f, 9.0f, estiloColores, 1, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(18.0f, 0.0f, -86.0f), 0.0f, 50.0f, 11.0f, estiloColores, 6, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(42.0f, 0.0f, -74.0f), -90.0f, 28.0f, 12.0f, estiloColores, 2, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(68.0f, 0.0f, -74.0f), 180.0f, 14.0f, 12.0f, estiloColores, 5, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(68.0f, 0.0f, -44.0f), 180.0f, 50.0f, 9.0f, estiloColores, 3, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(18.0f, 0.0f, -26.0f), 0.0f, 50.0f, 9.0f, estiloColores, 1, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(68.0f, 0.0f, -14.0f), 180.0f, 50.0f, 12.0f, estiloColores, 6, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(18.0f, 0.0f, 30.0f), 0.0f, 50.0f, 12.0f, estiloCantera, 4, uniformModel, uniformColor);

	//cierra la vista al norte
	DibujarFilaCasas(glm::vec3(-54.0f, 0.0f, -190.0f), 0.0f, 162.0f, 12.0f, estiloColores, 0, uniformModel, uniformColor);

	//portales del Zocalo al poniente, oriente y sur
	DibujarFilaCasas(glm::vec3(-35.0f, 0.0f, 112.0f), 90.0f, 70.0f, 20.0f, estiloPortal, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(35.0f, 0.0f, 42.0f), -90.0f, 70.0f, 35.0f, estiloPortal, 0, uniformModel, uniformColor);
	DibujarFilaCasas(glm::vec3(70.0f, 0.0f, 112.0f), 180.0f, 126.0f, 16.0f, estiloPortal, 0, uniformModel, uniformColor);

	//relleno del centro de cada manzana, mas bajo que las casas
	//queda 6 m adentro de la orilla, escondido dentro de las casas
	//las dos primeras no llevan relleno
	plainTexture.UseTexture();
	for (int i = 2; i < numManzanas; i++)
	{
		const Rectangulo& m = manzanas[i];
		glm::vec3 centro = glm::vec3((m.x0 + m.x1) / 2.0f, 0.0f, (m.z0 + m.z1) / 2.0f);
		glm::vec3 tamano = glm::vec3(m.x1 - m.x0 - 12.0f, 4.5f, m.z1 - m.z0 - 12.0f);
		DibujarBloque(centro, tamano, glm::vec3(0.74f, 0.70f, 0.64f), uniformModel, uniformColor);
	}
}


//dibuja una pieza con el cubo
//articulacion es el punto donde gira, centro es el centro del cubo desde ahi
//la escala solo va en esta copia, para no deformar a las partes hijas
void DibujarParte(glm::mat4 articulacion, glm::vec3 centro, glm::vec3 tamano, glm::vec3 color, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model = glm::translate(articulacion, centro);
	model = glm::scale(model, tamano);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshListModel[2]->RenderMeshModel();
}

//estanteria de madera de dos escalones pegada a una fachada, con tres macetas en cada escalon
//posicion es el centro de su orilla de atras, junto al muro, y con giro 0 el frente ve hacia Z positivo
//primera es el modelo de maceta con el que empieza, para que no todas las estanterias sean iguales
void DibujarEstanteMacetas(glm::vec3 posicion, float giro, int primera, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 estante(1.0);
	estante = glm::translate(estante, posicion);
	estante = glm::rotate(estante, giro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 blanco = glm::vec3(1.0f, 1.0f, 1.0f);

	//el escalon de enfrente es el bajo y el de atras el alto, asi las macetas de atras se asoman
	maderaTexture.UseTexture();
	DibujarParte(estante, glm::vec3(0.0f, 0.38f, 0.45f), glm::vec3(1.6f, 0.04f, 0.3f), blanco, uniformModel, uniformColor);
	DibujarParte(estante, glm::vec3(0.0f, 0.83f, 0.15f), glm::vec3(1.6f, 0.04f, 0.3f), blanco, uniformModel, uniformColor);
	for (int lado = -1; lado <= 1; lado += 2)
	{
		DibujarParte(estante, glm::vec3(lado * 0.77f, 0.2f, 0.57f), glm::vec3(0.06f, 0.4f, 0.06f), blanco, uniformModel, uniformColor);
		DibujarParte(estante, glm::vec3(lado * 0.77f, 0.425f, 0.03f), glm::vec3(0.06f, 0.85f, 0.06f), blanco, uniformModel, uniformColor);
		DibujarParte(estante, glm::vec3(lado * 0.77f, 0.3f, 0.3f), glm::vec3(0.04f, 0.04f, 0.54f), blanco, uniformModel, uniformColor);
	}

	for (int i = 0; i < 6; i++)
	{
		float x = (i % 3 - 1) * 0.5f;
		float y = 0.4f;
		float z = 0.45f;
		if (i >= 3)
		{
			y = 0.85f;
			z = 0.15f;
		}
		glm::mat4 model = glm::translate(estante, glm::vec3(x, y, z));
		model = glm::rotate(model, (i * 75.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		macetas[(primera + i) % numModelosMaceta].RenderModel();
	}
}

//llena una jardinera con flores sueltas cada tantos metros, sin pegarlas a la orilla ni a los arboles
//guarda las posiciones en flores a partir de cuantas y regresa cuantas hay ahora
int LlenarJardinera(Rectangulo r, float paso, glm::vec3 arboles[], int numArboles, glm::vec3 flores[], int cuantas, int maximo)
{
	float y = altoBanqueta + altoJardinera + 0.01f;
	for (float x = r.x0 + orillaJardinera + 0.6f; x < r.x1 - orillaJardinera - 0.3f; x += paso)
	{
		for (float z = r.z0 + orillaJardinera + 0.6f; z < r.z1 - orillaJardinera - 0.3f; z += paso)
		{
			bool cercaDeArbol = false;
			for (int i = 0; i < numArboles; i++)
			{
				if (glm::distance(glm::vec2(x, z), glm::vec2(arboles[i].x, arboles[i].z)) < 1.5f)
				{
					cercaDeArbol = true;
				}
			}
			if (!cercaDeArbol && cuantas < maximo)
			{
				flores[cuantas] = glm::vec3(x, y, z);
				cuantas++;
			}
		}
	}
	return cuantas;
}


// ================== 4. PERSONAJES DEL DESFILE ==================
//cada pieza se dibuja desde la matriz de su padre, asi al girar el hombro se mueve el antebrazo
//todos miran hacia Z positivo y cada pieza tiene su origen en la articulacion donde gira

// ---------- KeyFrames ----------
//como en el laboratorio, se guardan cuadros clave y en medio se interpola en linea recta
//cada cuadro guarda tres valores: en la china son los angulos del brazo que lanza el dulce
//y en la marmota su giro y lo que se inclina el palo
//al llegar al ultimo cuadro vuelve a empezar, por eso el ultimo es igual al primero
#define MAX_FRAMES 10
typedef struct _frame
{
	float valor[3];
	float valorInc[3];	//lo que cambia cada valor por paso hasta llegar al siguiente cuadro
} FRAME;

struct AnimacionKeyFrames
{
	FRAME KeyFrame[MAX_FRAMES];
	int FrameIndex;		//cuadros guardados
	int playIndex;		//cuadro del que va saliendo
	float i_curr_steps;
	float i_max_steps;	//pasos de un cuadro al siguiente, 60 son como un segundo
	float actual[3];	//valores con los que se dibuja
};

const int numChinas = 4;
AnimacionKeyFrames animChina[numChinas];
AnimacionKeyFrames animMarmota;

//cada china suelta un dulce por vuelta de su animacion y el dulce vuela un segundo
bool dulceLanzado[numChinas];
float tiempoDulce[numChinas];
const float vueloDulce = 1.0f;

void GuardarCuadro(AnimacionKeyFrames& anim, float valor0, float valor1, float valor2)
{
	if (anim.FrameIndex < MAX_FRAMES)
	{
		anim.KeyFrame[anim.FrameIndex].valor[0] = valor0;
		anim.KeyFrame[anim.FrameIndex].valor[1] = valor1;
		anim.KeyFrame[anim.FrameIndex].valor[2] = valor2;
		anim.FrameIndex++;
	}
}

void interpolation(AnimacionKeyFrames& anim)
{
	for (int k = 0; k < 3; k++)
	{
		anim.KeyFrame[anim.playIndex].valorInc[k] =
			(anim.KeyFrame[anim.playIndex + 1].valor[k] - anim.KeyFrame[anim.playIndex].valor[k]) / anim.i_max_steps;
	}
}

//primero es el cuadro con el que empieza, para que dos personajes iguales no se muevan al mismo tiempo
void IniciarAnimacion(AnimacionKeyFrames& anim, float pasos, int primero)
{
	anim.i_max_steps = pasos;
	anim.playIndex = primero;
	anim.i_curr_steps = 0.0f;
	for (int k = 0; k < 3; k++)
	{
		anim.actual[k] = anim.KeyFrame[primero].valor[k];
	}
	interpolation(anim);
}

void animate(AnimacionKeyFrames& anim, GLfloat deltaTime)
{
	if (anim.i_curr_steps >= anim.i_max_steps)
	{
		//al terminar el tramo se pone justo en el cuadro siguiente
		anim.playIndex++;
		if (anim.playIndex > anim.FrameIndex - 2)
		{
			anim.playIndex = 0;
		}
		for (int k = 0; k < 3; k++)
		{
			anim.actual[k] = anim.KeyFrame[anim.playIndex].valor[k];
		}
		anim.i_curr_steps = 0.0f;
		interpolation(anim);
	}
	else
	{
		for (int k = 0; k < 3; k++)
		{
			anim.actual[k] += anim.KeyFrame[anim.playIndex].valorInc[k] * deltaTime;
		}
		anim.i_curr_steps += deltaTime;
	}
}

//la china suelta el dulce al salir del cuadro 1, cuando el brazo deja la canasta
void ActualizarDulces(GLfloat segundos)
{
	for (int i = 0; i < numChinas; i++)
	{
		if (animChina[i].playIndex == 1 && !dulceLanzado[i])
		{
			dulceLanzado[i] = true;
			tiempoDulce[i] = 0.0f;
		}
		if (animChina[i].playIndex == 0)
		{
			dulceLanzado[i] = false;
		}
		tiempoDulce[i] += segundos;
	}
}

// ---------- hombre por piezas ----------
//angulos de un brazo en grados; con el brazo colgando todos valen 0
//adelante sube el brazo al frente, abre lo separa del cuerpo hacia el lado y con negativo lo cruza,
//codo dobla el antebrazo, giroMano lo gira sobre el codo y tuerce voltea la palma
struct PoseBrazo
{
	float adelante, abre, codo, giroMano, tuerce;
};

//la cadera con positivo lleva la pierna al frente y la rodilla con positivo dobla hacia atras
struct PosePersona
{
	PoseBrazo brazoIzq, brazoDer;
	float caderaIzq, rodillaIzq, caderaDer, rodillaDer;
	float cabezaGiro, cabezaSube;
	float altoCintura;	//sube y baja un poco al caminar
};

const PosePersona poseParado = { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f };

//posicion de cada articulacion respecto a la de su padre, en metros
const float zCintura = 0.061f;
const glm::vec3 posCuello = glm::vec3(0.0f, 0.56f, -0.078f);
const glm::vec3 posHombroIzq = glm::vec3(0.17f, 0.46f, -0.077f);
const glm::vec3 posHombroDer = glm::vec3(-0.17f, 0.46f, -0.092f);
const glm::vec3 posCodoIzq = glm::vec3(0.162f, -0.24f, -0.034f);
const glm::vec3 posCodoDer = glm::vec3(-0.162f, -0.24f, -0.022f);
const glm::vec3 posCaderaIzq = glm::vec3(0.1f, 0.0f, 0.014f);
const glm::vec3 posCaderaDer = glm::vec3(-0.1f, 0.0f, -0.005f);
const glm::vec3 posRodillaIzq = glm::vec3(0.01f, -0.42f, -0.045f);
const glm::vec3 posRodillaDer = glm::vec3(-0.01f, -0.42f, -0.032f);
//del codo a la palma de cada antebrazo, sobre esta linea se tuerce la mano
const glm::vec3 palmaIzq = glm::vec3(0.2247f, -0.2715f, 0.0761f);
const glm::vec3 palmaDer = glm::vec3(-0.2278f, -0.2714f, 0.0777f);
//los brazos del modelo vienen abiertos 34 grados, con este giro quedan colgando
const float aperturaModelo = 34.0f;

//ropa del personaje que se esta dibujando; con nullptr cada pieza usa la textura de su modelo
Texture* ropaActual = nullptr;

void DibujarPieza(Model& pieza, glm::mat4 model, GLuint uniformModel)
{
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	if (ropaActual == nullptr)
	{
		pieza.RenderModel();
	}
	else
	{
		pieza.RenderModel(*ropaActual);
	}
}

//la china usa los mismos brazos un poco mas chicos y en otro hombro
void DibujarBrazo(glm::mat4 torso, glm::vec3 hombro, bool izquierdo, PoseBrazo pose, float escala, GLuint uniformModel)
{
	//el izquierdo se abre hacia X positivo y el derecho hacia X negativo
	float lado = 1.0f;
	Model* brazo = &hombreBrazoIzq;
	Model* antebrazo = &hombreAntebrazoIzq;
	glm::vec3 codo = posCodoIzq;
	glm::vec3 palma = palmaIzq;
	if (!izquierdo)
	{
		lado = -1.0f;
		brazo = &hombreBrazoDer;
		antebrazo = &hombreAntebrazoDer;
		codo = posCodoDer;
		palma = palmaDer;
	}

	glm::mat4 model = glm::translate(torso, hombro);
	model = glm::rotate(model, -pose.adelante * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, lado * (pose.abre - aperturaModelo) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(escala, escala, escala));
	DibujarPieza(*brazo, model, uniformModel);

	model = glm::translate(model, codo);
	model = glm::rotate(model, pose.giroMano * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -pose.codo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, pose.tuerce * toRadians, glm::normalize(palma));
	DibujarPieza(*antebrazo, model, uniformModel);
}

void DibujarPierna(glm::mat4 torso, bool izquierda, float cadera, float rodilla, GLuint uniformModel)
{
	Model* muslo = &hombreMusloIzq;
	Model* pierna = &hombrePiernaIzq;
	glm::vec3 posCadera = posCaderaIzq;
	glm::vec3 posRodilla = posRodillaIzq;
	if (!izquierda)
	{
		muslo = &hombreMusloDer;
		pierna = &hombrePiernaDer;
		posCadera = posCaderaDer;
		posRodilla = posRodillaDer;
	}

	glm::mat4 model = glm::translate(torso, posCadera);
	model = glm::rotate(model, -cadera * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	DibujarPieza(*muslo, model, uniformModel);

	model = glm::translate(model, posRodilla);
	model = glm::rotate(model, rodilla * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	DibujarPieza(*pierna, model, uniformModel);
}

//base es la posicion y el giro del personaje en el piso
//regresa en torso y cabeza sus matrices, para colgarles la marmota, el torito o la trompeta
void DibujarHombre(glm::mat4 base, PosePersona pose, glm::mat4& torso, glm::mat4& cabeza, GLuint uniformModel)
{
	torso = glm::translate(base, glm::vec3(0.0f, pose.altoCintura, zCintura));
	//con otra ropa va el torso sin paliacate, que tiene su propia textura
	if (ropaActual == nullptr)
	{
		DibujarPieza(hombreTorso, torso, uniformModel);
	}
	else
	{
		DibujarPieza(transeunteTorso, torso, uniformModel);
	}

	cabeza = glm::translate(torso, posCuello);
	cabeza = glm::rotate(cabeza, pose.cabezaGiro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	cabeza = glm::rotate(cabeza, -pose.cabezaSube * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	DibujarPieza(hombreCabeza, cabeza, uniformModel);

	DibujarBrazo(torso, posHombroIzq, true, pose.brazoIzq, 1.0f, uniformModel);
	DibujarBrazo(torso, posHombroDer, false, pose.brazoDer, 1.0f, uniformModel);
	DibujarPierna(torso, true, pose.caderaIzq, pose.rodillaIzq, uniformModel);
	DibujarPierna(torso, false, pose.caderaDer, pose.rodillaDer, uniformModel);
}

//animacion de caminar; fase da una vuelta completa, 2 pi, cada dos pasos
//paso es cuanto se mueven los muslos, en grados; los brazos no se mueven porque todos cargan algo
void PoseCaminar(PosePersona& pose, float fase, float paso)
{
	float s = sin(fase);
	pose.caderaIzq = paso * s;
	pose.caderaDer = -paso * s;
	//la rodilla se dobla cuando la pierna va de regreso
	pose.rodillaIzq = 30.0f * glm::max(0.0f, sin(fase + 1.2f));
	pose.rodillaDer = 30.0f * glm::max(0.0f, sin(fase + 1.2f + 3.1416f));
	pose.altoCintura = 0.9f + 0.015f * cos(2.0f * fase);
}

//para pasar de una pose del brazo a otra; f va de 0 a 1
PoseBrazo MezclarBrazos(PoseBrazo a, PoseBrazo b, float f)
{
	PoseBrazo mezcla;
	mezcla.adelante = a.adelante + (b.adelante - a.adelante) * f;
	mezcla.abre = a.abre + (b.abre - a.abre) * f;
	mezcla.codo = a.codo + (b.codo - a.codo) * f;
	mezcla.giroMano = a.giroMano + (b.giroMano - a.giroMano) * f;
	mezcla.tuerce = a.tuerce + (b.tuerce - a.tuerce) * f;
	return mezcla;
}

// ---------- personajes ----------
//las poses de los brazos que agarran algo se buscaron para que las manos queden encima de lo que agarran

//el danzante agarra el palo con las dos manos, la derecha mas arriba, y la marmota gira con KeyFrames
const PoseBrazo brazoIzqMarmota = { 18.0f, -42.0f, 58.0f, 0.0f, 90.0f };
const PoseBrazo brazoDerMarmota = { 44.0f, -62.0f, 128.0f, 0.0f, 90.0f };
void DibujarDanzanteMarmota(glm::mat4 base, float fase, bool caminando, GLuint uniformModel)
{
	PosePersona pose = poseParado;
	if (caminando)
	{
		PoseCaminar(pose, fase, 18.0f);
	}
	pose.brazoIzq = brazoIzqMarmota;
	pose.brazoDer = brazoDerMarmota;
	glm::mat4 torso, cabeza;
	DibujarHombre(base, pose, torso, cabeza, uniformModel);

	//el palo sale de la cintura, al frente y un poco a la derecha
	glm::mat4 model = glm::translate(torso, glm::vec3(-0.08f, 0.05f, 0.28f));
	model = glm::rotate(model, animMarmota.actual[1] * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	DibujarPieza(marmotaPalo, model, uniformModel);
	model = glm::translate(model, glm::vec3(0.0f, 1.8f, 0.0f));
	model = glm::rotate(model, animMarmota.actual[0] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	DibujarPieza(marmota, model, uniformModel);
}

//la china sostiene la canasta con el brazo izquierdo y lanza dulces con el derecho, que se mueve con KeyFrames
const PoseBrazo brazoIzqChina = { 10.0f, 150.0f, 45.0f, 0.0f, 0.0f };
const glm::vec3 salidaDulce = glm::vec3(-0.422f, 1.755f, 0.027f);	//la mano derecha al soltarlo
void DibujarChina(glm::mat4 base, int cual, float tiempo, GLuint uniformModel)
{
	glm::mat4 torso = glm::translate(base, glm::vec3(0.0f, 1.1f, 0.0f));
	DibujarPieza(chinaTorso, torso, uniformModel);

	//la falda se mece de lado a lado y se abre un poco, una vez cada 2.4 segundos
	float vaiven = (tiempo + cual * 0.6f) * 2.0f * 3.1416f / 2.4f;
	glm::mat4 model = glm::rotate(torso, 6.0f * sin(vaiven) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, 3.0f * sin(2.0f * vaiven) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	float abre = 1.0f + 0.04f * sin(2.0f * vaiven);
	model = glm::scale(model, glm::vec3(abre, 1.0f, abre));
	ropaActual = &faldaChinaTexture[cual];
	DibujarPieza(chinaFalda, model, uniformModel);
	ropaActual = nullptr;

	model = glm::translate(torso, glm::vec3(0.0f, 0.521f, 0.0f));
	DibujarPieza(chinaCanasta, model, uniformModel);

	PoseBrazo brazoDer = { animChina[cual].actual[0], animChina[cual].actual[1], animChina[cual].actual[2], 0.0f, 0.0f };
	DibujarBrazo(torso, glm::vec3(0.155f, 0.2f, 0.0f), true, brazoIzqChina, 0.9f, uniformModel);
	DibujarBrazo(torso, glm::vec3(-0.155f, 0.2f, 0.0f), false, brazoDer, 0.9f, uniformModel);

	//el dulce cae en una parabola hacia el frente y a la derecha de la china, dando vueltas
	if (dulceLanzado[cual] && tiempoDulce[cual] < vueloDulce)
	{
		float s = tiempoDulce[cual] / vueloDulce;
		glm::vec3 vuelo = glm::vec3(-1.2f * s, 1.0f * s - 2.6f * s * s, 0.6f * s);
		model = glm::translate(base, salidaDulce + vuelo);
		model = glm::rotate(model, 12.0f * s, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarPieza(dulce, model, uniformModel);
	}
}

//las manos en la trompeta con la cabeza derecha y con la cabeza levantada; en medio se mezclan
const PoseBrazo brazoDerTrompeta[2] = { { 80.0f, -52.0f, 84.0f, 0.0f, -45.0f }, { 90.0f, -46.0f, 76.0f, 0.0f, -45.0f } };
const PoseBrazo brazoIzqTrompeta[2] = { { 68.0f, -64.0f, 110.0f, 0.0f, -45.0f }, { 76.0f, -64.0f, 110.0f, 0.0f, -45.0f } };
void DibujarMusico(glm::mat4 base, float fase, bool caminando, float tiempo, GLuint uniformModel)
{
	PosePersona pose = poseParado;
	if (caminando)
	{
		PoseCaminar(pose, fase, 15.0f);
	}
	//levanta la trompeta una vez cada 2 segundos
	float sube = glm::max(0.0f, sin(tiempo * 3.1416f));
	pose.cabezaSube = 10.0f * sube;
	pose.brazoDer = MezclarBrazos(brazoDerTrompeta[0], brazoDerTrompeta[1], sube);
	pose.brazoIzq = MezclarBrazos(brazoIzqTrompeta[0], brazoIzqTrompeta[1], sube);
	glm::mat4 torso, cabeza;
	DibujarHombre(base, pose, torso, cabeza, uniformModel);

	//la boquilla queda en la boca
	glm::mat4 model = glm::translate(cabeza, glm::vec3(0.0f, 0.055f, 0.135f));
	model = glm::rotate(model, 12.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	DibujarPieza(trompeta, model, uniformModel);
}

//el cargador lleva el torito sobre la cabeza, con cada mano en un palo del armazon
const PoseBrazo brazoIzqTorito = { 140.0f, -18.0f, 48.0f, 0.0f, 0.0f };
const PoseBrazo brazoDerTorito = { 138.0f, -18.0f, 46.0f, 0.0f, 0.0f };
void DibujarTorito(glm::mat4 base, float fase, bool caminando, float tiempo, GLuint uniformModel)
{
	PosePersona pose = poseParado;
	if (caminando)
	{
		PoseCaminar(pose, fase, 22.0f);
	}
	pose.brazoIzq = brazoIzqTorito;
	pose.brazoDer = brazoDerTorito;
	glm::mat4 torso, cabeza;
	DibujarHombre(base, pose, torso, cabeza, uniformModel);

	//el torito se mece de lado a lado
	glm::mat4 cuerpo = glm::translate(torso, glm::vec3(0.0f, 0.98f, 0.1f));
	cuerpo = glm::rotate(cuerpo, 8.0f * sin(tiempo * 3.1416f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	cuerpo = glm::rotate(cuerpo, 4.0f * sin(tiempo * 2.0f * 3.1416f) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	DibujarPieza(torito, cuerpo, uniformModel);

	//una rueda de cohetes a cada lado, dan una vuelta por segundo y cada una gira al reves de la otra
	for (int lado = -1; lado <= 1; lado += 2)
	{
		glm::mat4 rueda = glm::translate(cuerpo, glm::vec3(lado * 0.34f, 1.0f, -0.02f));
		rueda = glm::rotate(rueda, -lado * tiempo * 360.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		DibujarPieza(toritoRueda, rueda, uniformModel);

		//cuatro chispas en el aro, cambian de tamano para que parezca que chisporrotean
		for (int k = 0; k < 4; k++)
		{
			float angulo = k * 90.0f * toRadians;
			float tamano = 0.7f + 0.5f * fabs(sin(tiempo * 12.0f + k * 3.0f));
			glm::mat4 model = glm::translate(rueda, glm::vec3(0.0f, 0.22f * sin(angulo), -0.22f * cos(angulo)));
			model = glm::scale(model, glm::vec3(tamano, tamano, tamano));
			DibujarPieza(toritoChispa, model, uniformModel);
		}
	}
}

//los espectadores saludan con el brazo derecho; la mano va y viene dos veces por cada vez que voltean
PoseBrazo PoseSaludo(float vuelta)
{
	PoseBrazo saludo = { 10.0f, 130.0f, 20.0f, 30.0f * sin(2.0f * vuelta), 0.0f };
	return saludo;
}

//el hombre usa las piezas del desfile con otra ropa y voltea la cabeza
void DibujarEspectador(glm::mat4 base, float tiempo, int ropa, GLuint uniformModel)
{
	float vuelta = tiempo * 3.1416f;
	PosePersona pose = poseParado;
	pose.brazoDer = PoseSaludo(vuelta);
	pose.cabezaGiro = 15.0f * sin(vuelta);
	glm::mat4 torso, cabeza;
	ropaActual = &ropaHombreTexture[ropa];
	DibujarHombre(base, pose, torso, cabeza, uniformModel);
	ropaActual = nullptr;
}

//la mujer usa el torso de la china con otra blusa, los brazos y las pantorrillas del hombre y una falda lisa
//como su cabeza va pegada al torso, voltea todo el cuerpo de la cintura para arriba
const glm::vec3 coloresFaldaMujer[numRopas] = {
	glm::vec3(0.2f, 0.25f, 0.45f), glm::vec3(0.2f, 0.2f, 0.22f), glm::vec3(0.45f, 0.3f, 0.2f), glm::vec3(0.2f, 0.4f, 0.3f)
};
const PoseBrazo brazoColgando = { 0.0f, 12.0f, 10.0f, 0.0f, 0.0f };
void DibujarEspectadora(glm::mat4 base, float tiempo, int ropa, GLuint uniformModel, GLuint uniformColor)
{
	float vuelta = tiempo * 3.1416f;
	ropaActual = &ropaMujerTexture[ropa];
	//las rodillas quedan escondidas bajo la falda, que llega a 40 cm del piso
	glm::mat4 model = glm::translate(base, glm::vec3(0.09f, 0.432f, 0.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	DibujarPieza(hombrePiernaIzq, model, uniformModel);
	model = glm::translate(base, glm::vec3(-0.09f, 0.432f, 0.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	DibujarPieza(hombrePiernaDer, model, uniformModel);

	glm::mat4 cintura = glm::translate(base, glm::vec3(0.0f, 1.1f, 0.0f));
	glm::mat4 torso = glm::rotate(cintura, 10.0f * sin(vuelta) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	DibujarBrazo(torso, glm::vec3(0.155f, 0.2f, 0.0f), true, brazoColgando, 0.9f, uniformModel);
	DibujarBrazo(torso, glm::vec3(-0.155f, 0.2f, 0.0f), false, PoseSaludo(vuelta), 0.9f, uniformModel);
	ropaActual = &blusaMujerTexture[ropa];
	DibujarPieza(chinaTorso, torso, uniformModel);
	ropaActual = nullptr;

	glUniform3fv(uniformColor, 1, glm::value_ptr(coloresFaldaMujer[ropa]));
	DibujarPieza(mujerFalda, cintura, uniformModel);
	glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
}

//tres globos que suben y bajan junto al espectador
const glm::vec3 coloresGlobo[3] = {
	glm::vec3(0.9f, 0.2f, 0.3f), glm::vec3(0.2f, 0.5f, 0.9f), glm::vec3(0.95f, 0.8f, 0.2f)
};
void DibujarGlobos(glm::mat4 base, float tiempo, GLuint uniformModel, GLuint uniformColor)
{
	for (int i = 0; i < 3; i++)
	{
		//cada globo sube 2 m y vuelve a bajar en 6 segundos, cada uno en otro momento
		float s = 0.5f - 0.5f * cos(2.0f * 3.1416f * (tiempo / 6.0f + i / 3.0f));
		glm::mat4 model = glm::translate(base, glm::vec3(0.7f + 0.25f * i + 0.1f * sin(4.0f * 3.1416f * s), 0.6f + 2.0f * s, 0.4f * i - 0.4f));
		model = glm::rotate(model, 8.0f * sin(2.0f * 3.1416f * (2.0f * s + i)) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniform3fv(uniformColor, 1, glm::value_ptr(coloresGlobo[i]));
		DibujarPieza(globo, model, uniformModel);
	}
	glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
}

// ---------- recorrido del desfile ----------
//sale del atrio frente a la puerta de Santo Domingo hacia el poniente, da vuelta al sur en el andador
//y baja hasta quedar frente al kiosco; los puntos son (x, z)
const int numPuntosRecorrido = 4;
const glm::vec2 puntosRecorrido[numPuntosRecorrido] = {
	glm::vec2(50.0f, -139.5f), glm::vec2(6.0f, -139.5f), glm::vec2(0.0f, -133.5f), glm::vec2(0.0f, 63.0f)
};
const float velocidadDesfile = 1.3f;	//metros por segundo
const float zancada = 1.5f;				//lo que avanza un personaje con dos pasos
const float esperaDesfile = 8.0f;		//segundos frente al kiosco antes de volver a empezar

//personajes en el orden de la fila; atras es cuantos metros va detras del primero
//y lado cuanto se hace hacia su izquierda
const int MARMOTA = 0;
const int CHINA = 1;
const int MUSICO = 2;
const int TORITO = 3;
const int numDesfile = 9;
const int tipoDesfile[numDesfile] = { MARMOTA, CHINA, CHINA, CHINA, CHINA, MUSICO, MUSICO, MUSICO, TORITO };
const float atrasDesfile[numDesfile] = { 0.0f, 3.5f, 3.5f, 7.0f, 7.0f, 11.0f, 11.0f, 11.0f, 15.5f };
const float ladoDesfile[numDesfile] = { 0.0f, 1.2f, -1.2f, 1.2f, -1.2f, 1.6f, 0.0f, -1.6f, 0.0f };

float LargoRecorrido()
{
	float largo = 0.0f;
	for (int i = 0; i < numPuntosRecorrido - 1; i++)
	{
		largo += glm::distance(puntosRecorrido[i], puntosRecorrido[i + 1]);
	}
	return largo;
}

//punto (x, z) a cierta distancia del inicio; antes del inicio o despues del final se queda en la orilla
glm::vec2 PuntoRecorrido(float distancia)
{
	for (int i = 0; i < numPuntosRecorrido - 1; i++)
	{
		float tramo = glm::distance(puntosRecorrido[i], puntosRecorrido[i + 1]);
		if (distancia <= tramo)
		{
			float f = glm::max(distancia, 0.0f) / tramo;
			return puntosRecorrido[i] + (puntosRecorrido[i + 1] - puntosRecorrido[i]) * f;
		}
		distancia -= tramo;
	}
	return puntosRecorrido[numPuntosRecorrido - 1];
}

//hacia donde camina, viendo medio metro atras y medio adelante; asi da la vuelta poco a poco en la esquina
//con giro 0 ve hacia Z positivo
float GiroRecorrido(float distancia)
{
	glm::vec2 direccion = PuntoRecorrido(distancia + 0.5f) - PuntoRecorrido(distancia - 0.5f);
	return atan2(direccion.x, direccion.y) / toRadians;
}

//las plazas y las banquetas estan 15 cm arriba de la calle y el andador 1 cm
float AlturaPiso(float x, float z)
{
	for (int i = 0; i < numPlazas; i++)
	{
		if (x >= plazas[i].x0 && x <= plazas[i].x1 && z >= plazas[i].z0 && z <= plazas[i].z1)
		{
			return altoBanqueta;
		}
	}
	for (int i = 0; i < numManzanas; i++)
	{
		if (x >= manzanas[i].x0 && x <= manzanas[i].x1 && z >= manzanas[i].z0 && z <= manzanas[i].z1)
		{
			return altoBanqueta;
		}
	}
	return 0.01f;
}

//lado mueve el punto hacia la izquierda de quien camina, que con giro 0 es X positivo
glm::vec3 PosicionRecorrido(float distancia, float lado)
{
	glm::vec2 punto = PuntoRecorrido(distancia);
	float giro = GiroRecorrido(distancia) * toRadians;
	glm::vec2 izquierda = glm::vec2(cos(giro), -sin(giro));
	punto = punto + izquierda * lado;
	return glm::vec3(punto.x, AlturaPiso(punto.x, punto.y), punto.y);
}


int main()
{
	//el tamano lo toma del monitor en Initialise
	mainWindow = Window();
	mainWindow.Initialise();
	CreateObjects();
	CrearCubo();
	CrearEscenario();
	CrearPlano();
	CrearJardineras();
	CrearTiraPapelPicado();
	CreateShaders();

	//posicion, up, yaw, pitch, velocidad de movimiento y de giro
	//yaw 90 mira hacia Z positivo, 0 hacia X positivo
	//sobre el andador frente al atrio, a 10 m, mirando hacia el sur
	camera = Camera(glm::vec3(0.0f, 10.0f, -150.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -15.0f, 0.3f, 0.5f);
	//en el atrio, mira hacia el oriente a Santo Domingo, un poco arriba para ver las torres
	//velocidades en 0, no se mueve
	camaraFachada = Camera(glm::vec3(12.0f, 9.0f, -144.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 12.0f, 0.0f, 0.0f);
	//sigue al desfile desde un lado, su posicion se calcula en el while
	camaraDesfile = Camera(glm::vec3(5.0f, 2.6f, -112.0f), glm::vec3(0.0f, 1.0f, 0.0f), 110.0f, -8.0f, 0.0f, 0.0f);
	//esquina noroeste de la plaza, mira en diagonal hacia el kiosco
	camaraZocalo = Camera(glm::vec3(-30.0f, 10.0f, 46.0f), glm::vec3(0.0f, 1.0f, 0.0f), 46.0f, -8.0f, 0.0f, 0.0f);
	//a 300 m sobre el centro, mira hacia abajo con el norte arriba
	//pitch -89 y no -90, mirando derecho hacia abajo no se sabe cual es la derecha
	camaraAerea = Camera(glm::vec3(27.0f, 300.0f, -38.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -89.0f, 0.0f, 0.0f);
	//camara con la que se dibuja, cambia en el while
	Camera* camaraActual = &camera;

	// ================== 5. CARGA DE RECURSOS ==================

	//primero la ruta, luego se carga
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
	papelPicadoTexture = Texture("Textures/papel_picado.png");
	papelPicadoTexture.LoadTextureA();
	maderaTexture = Texture("Textures/madera.jpg");
	maderaTexture.LoadTexture();
	canteraTexture = Texture("Textures/cantera_muro.jpg");
	canteraTexture.LoadTexture();
	tierraTexture = Texture("Textures/tierra_roja.jpg");
	tierraTexture.LoadTexture();

	//las texturas de los modelos van en Textures/
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
	agave = Model();
	agave.LoadModel("Models/agave.obj");
	flamboyan = Model();
	flamboyan.LoadModel("Models/arbol_flamboyan.obj");
	palmera = Model();
	palmera.LoadModel("Models/palmera.obj");
	arbolZocalo1 = Model();
	arbolZocalo1.LoadModel("Models/arbol_igor_1.obj");
	arbolZocalo2 = Model();
	arbolZocalo2.LoadModel("Models/arbol_igor_2.obj");
	arbolLigero1 = Model();
	arbolLigero1.LoadModel("Models/arbol_ligero_1.obj");
	arbolLigero2 = Model();
	arbolLigero2.LoadModel("Models/arbol_ligero_2.obj");
	jacaranda = Model();
	jacaranda.LoadModel("Models/jacaranda.obj");
	cacaloxochitl = Model();
	cacaloxochitl.LoadModel("Models/cacaloxochitl.obj");
	const char* archivosMaceta[numModelosMaceta] = {
		"Models/maceta_margaritas.obj", "Models/maceta_planta_verde.obj", "Models/maceta_narcisos.obj",
		"Models/maceta_anemonas.obj", "Models/maceta_pasto.obj", "Models/maceta_violetas.obj"
	};
	for (int i = 0; i < numModelosMaceta; i++)
	{
		macetas[i] = Model();
		macetas[i].LoadModel(archivosMaceta[i]);
	}
	const char* archivosFlor[numModelosFlor] = {
		"Models/margarita_1.obj", "Models/narciso_amarillo.obj", "Models/anemona.obj",
		"Models/margarita_2.obj", "Models/pasto_paquete_1.obj", "Models/narciso_blanco.obj"
	};
	for (int i = 0; i < numModelosFlor; i++)
	{
		flores[i] = Model();
		flores[i].LoadModel(archivosFlor[i]);
	}
	banca = Model();
	banca.LoadModel("Models/banca.obj");
	carritoElotes = Model();
	carritoElotes.LoadModel("Models/carrito_elotes.obj");
	bugambilia = Model();
	bugambilia.LoadModel("Models/bugambilia.obj");
	bugambiliaMuro = Model();
	bugambiliaMuro.LoadModel("Models/bugambilia_muro.obj");
	mesaToldo = Model();
	mesaToldo.LoadModel("Models/mesa_toldo.obj");
	tendedero = Model();
	tendedero.LoadModel("Models/tendedero.obj");
	canasta = Model();
	canasta.LoadModel("Models/canasta.obj");
	petate = Model();
	petate.LoadModel("Models/petate.obj");
	cazuela = Model();
	cazuela.LoadModel("Models/cazuela.obj");
	olla = Model();
	olla.LoadModel("Models/olla.obj");
	taza = Model();
	taza.LoadModel("Models/taza_barro_cara.obj");
	panYema = Model();
	panYema.LoadModel("Models/pan_yema.obj");
	alebrijeCoyote = Model();
	alebrijeCoyote.LoadModel("Models/alebrije_coyote.obj");
	alebrijePajaro = Model();
	alebrijePajaro.LoadModel("Models/alebrije_pajaro.obj");
	alebrijePerrito = Model();
	alebrijePerrito.LoadModel("Models/perrito.obj");
	hombreTorso = Model();
	hombreTorso.LoadModel("Models/hombre_torso.obj");
	hombreCabeza = Model();
	hombreCabeza.LoadModel("Models/hombre_cabeza.obj");
	hombreBrazoIzq = Model();
	hombreBrazoIzq.LoadModel("Models/hombre_brazo_izq.obj");
	hombreAntebrazoIzq = Model();
	hombreAntebrazoIzq.LoadModel("Models/hombre_antebrazo_izq.obj");
	hombreBrazoDer = Model();
	hombreBrazoDer.LoadModel("Models/hombre_brazo_der.obj");
	hombreAntebrazoDer = Model();
	hombreAntebrazoDer.LoadModel("Models/hombre_antebrazo_der.obj");
	hombreMusloIzq = Model();
	hombreMusloIzq.LoadModel("Models/hombre_muslo_izq.obj");
	hombrePiernaIzq = Model();
	hombrePiernaIzq.LoadModel("Models/hombre_pierna_izq.obj");
	hombreMusloDer = Model();
	hombreMusloDer.LoadModel("Models/hombre_muslo_der.obj");
	hombrePiernaDer = Model();
	hombrePiernaDer.LoadModel("Models/hombre_pierna_der.obj");
	chinaTorso = Model();
	chinaTorso.LoadModel("Models/china_torso.obj");
	chinaFalda = Model();
	chinaFalda.LoadModel("Models/china_falda.obj");
	chinaCanasta = Model();
	chinaCanasta.LoadModel("Models/china_canasta.obj");
	dulce = Model();
	dulce.LoadModel("Models/dulce.obj");
	marmota = Model();
	marmota.LoadModel("Models/marmota.obj");
	marmotaPalo = Model();
	marmotaPalo.LoadModel("Models/marmota_palo.obj");
	torito = Model();
	torito.LoadModel("Models/torito.obj");
	toritoRueda = Model();
	toritoRueda.LoadModel("Models/torito_rueda.obj");
	toritoChispa = Model();
	toritoChispa.LoadModel("Models/torito_chispa.obj");
	trompeta = Model();
	trompeta.LoadModel("Models/trompeta.obj");
	globo = Model();
	globo.LoadModel("Models/globo.obj");
	transeunteTorso = Model();
	transeunteTorso.LoadModel("Models/transeunte_torso.obj");
	mujerFalda = Model();
	mujerFalda.LoadModel("Models/mujer_falda.obj");

	//Texture guarda el nombre sin copiarlo, por eso cada nombre se arma en su variable y se carga enseguida
	for (int i = 0; i < numRopas; i++)
	{
		std::string numero = std::to_string(i + 1);
		std::string falda = "Textures/china_falda_" + numero + ".jpg";
		std::string blusa = "Textures/blusa_mujer_" + numero + ".jpg";
		if (i == 0)
		{
			falda = "Textures/china_falda.jpg";
			blusa = "Textures/china_cuerpo.jpg";
		}
		std::string ropaHombre = "Textures/ropa_hombre_" + numero + ".jpg";
		std::string ropaMujer = "Textures/ropa_mujer_" + numero + ".jpg";
		faldaChinaTexture[i] = Texture(falda.c_str());
		faldaChinaTexture[i].LoadTexture();
		blusaMujerTexture[i] = Texture(blusa.c_str());
		blusaMujerTexture[i].LoadTexture();
		ropaHombreTexture[i] = Texture(ropaHombre.c_str());
		ropaHombreTexture[i].LoadTexture();
		ropaMujerTexture[i] = Texture(ropaMujer.c_str());
		ropaMujerTexture[i].LoadTexture();
	}

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/tarde_rt.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_lf.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_dn.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_up.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_bk.jpg");
	skyboxFaces.push_back("Textures/Skybox/tarde_ft.jpg");
	skyboxTarde = Skybox(skyboxFaces);

	//reusa la lista para las caras de la noche
	skyboxFaces.clear();
	skyboxFaces.push_back("Textures/Skybox/noche_rt.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_lf.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_dn.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_up.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_bk.jpg");
	skyboxFaces.push_back("Textures/Skybox/noche_ft.jpg");
	skyboxNoche = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	// ---------- luces ----------
	//orden de los parametros de cada tipo de luz
	//DirectionalLight recibe color, ambiental, difusa y direccion
	//PointLight recibe color, ambiental, difusa, posicion y atenuacion
	//SpotLight recibe lo de PointLight, la direccion despues de la posicion y al final el angulo del cono

	//siempre tiene que haber una luz direccional
	//sol naranja y bajo desde el oeste, le pega de frente a Santo Domingo
	luzTarde = DirectionalLight(1.0f, 0.75f, 0.5f,
		0.35f, 0.5f,
		1.0f, -0.4f, 0.3f);
	//luz azul tenue casi desde arriba, como la luna
	luzNoche = DirectionalLight(0.4f, 0.45f, 0.7f,
		0.12f, 0.15f,
		0.2f, -1.0f, 0.3f);

	//farolas en las banquetas del andador, cada 16 m mas o menos, y cuatro en el Zocalo
	//su base esta a la altura de la banqueta
	//las primeras 12 son de brazo y las ultimas 4 de globos
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
	//el farol cuelga 22 cm hacia X negativo, a 3.85 m de alto
	//las del poniente se giran 180 grados para que el farol quede hacia la calle
	float giroFarolas[numFarolas];
	glm::vec3 posicionFaroles[numFarolas];
	for (int i = 0; i < numFarolas; i++)
	{
		if (i >= numFarolasAndador)
		{
			//la luz va entre los dos globos
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
	//luz puntual amarilla en cada farola, alumbra un circulo de unos 6 m de radio
	for (int i = 0; i < numFarolas; i++)
	{
		pointLights[i] = PointLight(1.0f, 0.8f, 0.5f,
			0.0f, 1.2f,
			posicionFaroles[i].x, posicionFaroles[i].y, posicionFaroles[i].z,
			1.0f, 0.09f, 0.032f);
	}
	//en el while, 0 de tarde y numFarolas de noche
	unsigned int pointLightCount = 0;

	//no puede pasar de MAX_SPOT_LIGHTS
	unsigned int spotLightCount = 0;
	//sale del piso del atrio y apunta hacia la fachada
	spotLights[0] = SpotLight(1.0f, 0.9f, 0.75f,
		0.0f, 1.2f,
		36.0f, 0.5f, -142.0f,
		1.0f, 0.6f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		45.0f);
	spotLightCount++;

	//debajo del techo, apunta hacia abajo a la plataforma de la banda
	spotLights[1] = SpotLight(1.0f, 0.85f, 0.6f,
		0.0f, 1.5f,
		0.0f, 7.0f, 77.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		55.0f);
	spotLightCount++;

	//sale del piso frente a la puerta y apunta hacia el poniente
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

	//telones al final de cada calle que cruza, centro de su orilla de abajo
	const int numTelones = 8;
	glm::vec3 posicionTelones[numTelones] = {
		glm::vec3(-56.0f, 0.0f, -184.0f), glm::vec3(110.0f, 0.0f, -184.0f),	//Gurrion
		glm::vec3(-56.0f, 0.0f, -80.0f), glm::vec3(70.0f, 0.0f, -80.0f),		//Abasolo y Manuel Bravo
		glm::vec3(-56.0f, 0.0f, -20.0f), glm::vec3(70.0f, 0.0f, -20.0f),		//Morelos
		glm::vec3(-56.0f, 0.0f, 37.0f), glm::vec3(70.0f, 0.0f, 37.0f)			//Independencia
	};
	//el poniente ve hacia X positivo, el oriente hacia X negativo
	float giroTelones[numTelones] = { 90.0f, -90.0f, 90.0f, -90.0f, 90.0f, -90.0f, 90.0f, -90.0f };

	//cada 4 m, solo donde hay casas de los dos lados
	//cuatro entre Abasolo y Morelos, once entre Morelos e Independencia
	const int numPapelPicado = 15;
	float zPapelPicado[numPapelPicado] = {
		-42.0f, -38.0f, -34.0f, -30.0f,
		-12.0f, -8.0f, -4.0f, 0.0f, 4.0f, 8.0f, 12.0f, 16.0f, 20.0f, 24.0f, 28.0f
	};
	const float altoPapelPicado = 5.2f;		//altura del cordel, abajo de la casa mas baja

	//arboles del jardin Labastida, en el piso de la plaza
	const int numArbolesLabastida = 8;
	glm::vec3 posicionArbolesLabastida[numArbolesLabastida] = {
		glm::vec3(12.0f, altoBanqueta, -70.0f), glm::vec3(24.0f, altoBanqueta, -68.0f), glm::vec3(35.0f, altoBanqueta, -71.0f),
		glm::vec3(14.0f, altoBanqueta, -54.0f), glm::vec3(27.0f, altoBanqueta, -51.0f), glm::vec3(36.0f, altoBanqueta, -58.0f),
		glm::vec3(20.0f, altoBanqueta, -61.0f), glm::vec3(31.0f, altoBanqueta, -63.0f)
	};

	//jacarandas en la franja del atrio junto al andador, lejos de la camara de la fachada
	const int numArbolesAtrio = 2;
	glm::vec3 posicionArbolesAtrio[numArbolesAtrio] = {
		glm::vec3(10.0f, altoBanqueta, -166.0f), glm::vec3(10.0f, altoBanqueta, -127.0f)
	};

	//ocho sobre las jardineras, dos en cada una, y los demas en el piso
	//en las esquinas que no quedan en la diagonal al kiosco, para no tapar la vista
	//la esquina noroeste se deja libre por la camara del Zocalo
	const int numArbolesZocalo = 13;
	const float ySobreJardinera = altoBanqueta + altoJardinera + 0.01f;
	glm::vec3 posicionArbolesZocalo[numArbolesZocalo] = {
		glm::vec3(-26.0f, ySobreJardinera, 60.0f), glm::vec3(-14.0f, ySobreJardinera, 52.0f),
		glm::vec3(26.0f, ySobreJardinera, 60.0f), glm::vec3(14.0f, ySobreJardinera, 52.0f),
		glm::vec3(-26.0f, ySobreJardinera, 94.0f), glm::vec3(-14.0f, ySobreJardinera, 102.0f),
		glm::vec3(26.0f, ySobreJardinera, 94.0f), glm::vec3(14.0f, ySobreJardinera, 102.0f),
		glm::vec3(-31.0f, altoBanqueta, 77.0f), glm::vec3(31.0f, altoBanqueta, 77.0f),
		glm::vec3(31.0f, altoBanqueta, 50.0f), glm::vec3(-31.0f, altoBanqueta, 104.0f), glm::vec3(31.0f, altoBanqueta, 104.0f)
	};

	//flores en las cuatro jardineras del Zocalo y en la de los flamboyanes
	glm::vec3 posicionFlamboyanes[5];
	for (int i = 0; i < 5; i++)
	{
		posicionFlamboyanes[i] = glm::vec3(10.0f + i * 9.0f, ySobreJardinera, -176.75f);
	}
	const int maxFlores = 1200;
	glm::vec3 posicionFlores[maxFlores];
	int numFlores = 0;
	for (int i = 2; i < 6; i++)
	{
		numFlores = LlenarJardinera(jardineras[i], 0.9f, posicionArbolesZocalo, numArbolesZocalo, posicionFlores, numFlores, maxFlores);
	}
	numFlores = LlenarJardinera(jardineras[6], 0.9f, posicionFlamboyanes, 5, posicionFlores, numFlores, maxFlores);

	//banqueta poniente, frente al atrio
	const int numCacaloxochitl = 2;
	glm::vec3 posicionCacaloxochitl[numCacaloxochitl] = {
		glm::vec3(-5.0f, altoBanqueta, -160.0f), glm::vec3(-5.0f, altoBanqueta, -128.0f)
	};

	//estanterias de macetas pegadas a las fachadas, donde no estorban farolas ni cacaloxochitl
	//las del poniente ven hacia X positivo y las del oriente hacia X negativo
	const int numEstantes = 13;
	glm::vec3 posicionEstantes[numEstantes] = {
		glm::vec3(-5.97f, altoBanqueta, -167.0f), glm::vec3(-5.97f, altoBanqueta, -135.0f), glm::vec3(-5.97f, altoBanqueta, -103.0f),
		glm::vec3(-5.97f, altoBanqueta, -71.0f), glm::vec3(-5.97f, altoBanqueta, -55.0f), glm::vec3(-5.97f, altoBanqueta, -39.0f),
		glm::vec3(-5.97f, altoBanqueta, -11.0f), glm::vec3(-5.97f, altoBanqueta, 5.0f), glm::vec3(-5.97f, altoBanqueta, 21.0f),
		glm::vec3(5.97f, altoBanqueta, -103.0f), glm::vec3(5.97f, altoBanqueta, -41.0f), glm::vec3(5.97f, altoBanqueta, -11.0f),
		glm::vec3(5.97f, altoBanqueta, 13.0f)
	};

	//bancas del Zocalo, respaldo pegado a las jardineras
	//las seis primeras ven hacia el sur y las otras seis hacia el norte
	const int numBancas = 12;
	glm::vec3 posicionBancas[numBancas] = {
		glm::vec3(-26.0f, altoBanqueta, 63.5f), glm::vec3(-20.0f, altoBanqueta, 63.5f), glm::vec3(-14.0f, altoBanqueta, 63.5f),
		glm::vec3(14.0f, altoBanqueta, 63.5f), glm::vec3(20.0f, altoBanqueta, 63.5f), glm::vec3(26.0f, altoBanqueta, 63.5f),
		glm::vec3(-26.0f, altoBanqueta, 90.5f), glm::vec3(-20.0f, altoBanqueta, 90.5f), glm::vec3(-14.0f, altoBanqueta, 90.5f),
		glm::vec3(14.0f, altoBanqueta, 90.5f), glm::vec3(20.0f, altoBanqueta, 90.5f), glm::vec3(26.0f, altoBanqueta, 90.5f)
	};

	//junto a los portales, letrero hacia el kiosco
	const int numCarritos = 2;
	glm::vec3 posicionCarritos[numCarritos] = {
		glm::vec3(-32.0f, altoBanqueta, 70.0f), glm::vec3(32.0f, altoBanqueta, 84.0f)
	};
	float giroCarritos[numCarritos] = { 90.0f, -90.0f };

	//dos arbolitos en el sur del jardin Labastida
	//y cuatro que cuelgan de fachadas, 2 cm enfrente del muro
	const int numBugambilias = 2;
	glm::vec3 posicionBugambilias[numBugambilias] = {
		glm::vec3(10.0f, altoBanqueta, -49.0f), glm::vec3(37.0f, altoBanqueta, -49.0f)
	};
	const int numBugambiliasMuro = 4;
	glm::vec3 posicionBugambiliasMuro[numBugambiliasMuro] = {
		glm::vec3(-5.98f, 0.0f, -150.0f), glm::vec3(-5.98f, 0.0f, -60.0f), glm::vec3(-5.98f, 0.0f, -45.0f),
		glm::vec3(5.98f, 0.0f, -37.0f)
	};
	//las del poniente ven hacia X positivo, la del oriente hacia X negativo
	float giroBugambiliasMuro[numBugambiliasMuro] = { 90.0f, 90.0f, 90.0f, -90.0f };

	//puesto de textiles junto al andador, todo viendo hacia el andador
	//se turnan mesas con toldo y tendederos
	const int numPuestosTextiles = 6;
	float zPuestosTextiles[numPuestosTextiles] = { -73.0f, -69.5f, -66.0f, -62.5f, -59.0f, -55.5f };

	//espectadores en las banquetas y las plazas, viendo hacia donde pasa el desfile
	//los del poniente ven hacia X positivo, los del oriente hacia X negativo y los del atrio hacia el sur
	//uno si y otro no carga globos
	const int numEspectadores = 16;
	glm::vec3 posicionEspectadores[numEspectadores] = {
		glm::vec3(-4.7f, altoBanqueta, -120.0f), glm::vec3(-4.7f, altoBanqueta, -88.0f), glm::vec3(-4.7f, altoBanqueta, -50.0f),
		glm::vec3(-4.7f, altoBanqueta, -28.0f), glm::vec3(-4.7f, altoBanqueta, -2.0f), glm::vec3(-4.7f, altoBanqueta, 16.0f),
		glm::vec3(-4.7f, altoBanqueta, 28.0f),
		glm::vec3(4.7f, altoBanqueta, -118.0f), glm::vec3(4.7f, altoBanqueta, -90.0f), glm::vec3(4.7f, altoBanqueta, -30.0f),
		glm::vec3(4.7f, altoBanqueta, 2.0f), glm::vec3(4.7f, altoBanqueta, 20.0f),
		glm::vec3(26.0f, altoBanqueta, -144.5f), glm::vec3(36.0f, altoBanqueta, -144.5f),
		glm::vec3(-6.0f, altoBanqueta, 50.0f), glm::vec3(-6.0f, altoBanqueta, 58.0f)
	};
	float giroEspectadores[numEspectadores] = {
		90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f,
		-90.0f, -90.0f, -90.0f, -90.0f, -90.0f,
		0.0f, 0.0f,
		90.0f, 90.0f
	};
	//mujeres y hombres revueltos, cada uno con una de las cuatro ropas
	bool esMujer[numEspectadores] = {
		false, true, true, false, false, true, false,
		true, false, true, true, false,
		true, false,
		false, true
	};
	int ropaEspectadores[numEspectadores] = {
		0, 1, 3, 2, 1, 0, 3,
		2, 3, 0, 2, 0,
		1, 2,
		3, 3
	};

	//cuadros clave de la china: brazo abajo, mete la mano a la canasta, lanza el dulce al frente,
	//vuelve abajo y descansa; los valores son adelante, abre y codo del brazo derecho
	//cada china empieza en otro cuadro
	for (int i = 0; i < numChinas; i++)
	{
		GuardarCuadro(animChina[i], 10.0f, 15.0f, 15.0f);
		GuardarCuadro(animChina[i], 20.0f, 145.0f, 50.0f);
		GuardarCuadro(animChina[i], 45.0f, 60.0f, 10.0f);
		GuardarCuadro(animChina[i], 10.0f, 15.0f, 15.0f);
		GuardarCuadro(animChina[i], 10.0f, 15.0f, 15.0f);
		IniciarAnimacion(animChina[i], 36.0f, i);
	}
	//cuadros clave de la marmota: da una vuelta en cuatro partes mientras el palo se inclina a un lado y al otro
	//el tercer valor no se usa
	GuardarCuadro(animMarmota, 0.0f, 0.0f, 0.0f);
	GuardarCuadro(animMarmota, 90.0f, 4.0f, 0.0f);
	GuardarCuadro(animMarmota, 180.0f, 0.0f, 0.0f);
	GuardarCuadro(animMarmota, 270.0f, -4.0f, 0.0f);
	GuardarCuadro(animMarmota, 360.0f, 0.0f, 0.0f);
	IniciarAnimacion(animMarmota, 60.0f, 0);

	//el desfile empieza con el ultimo de la fila en el inicio del recorrido
	const float largoRecorrido = LargoRecorrido();
	const float inicioDesfile = atrasDesfile[numDesfile - 1];
	float avanceDesfile = inicioDesfile;	//metros que lleva el primero de la fila
	//reloj propio del desfile, se detiene con la pausa para que todo se quede quieto
	float tiempoDesfile = 0.0f;
	//true en el frame en que el desfile sale del atrio y cuando llega frente al kiosco
	bool empiezaDesfile = true;
	bool desfileLlego = false;

	//---------- audio ----------
	//el motor de miniaudio manda todo a las bocinas; si no hay bocinas el programa sigue sin sonido
	ma_engine motorAudio;
	bool hayAudio = ma_engine_init(NULL, &motorAudio) == MA_SUCCESS;
	//la banda toca sus canciones una tras otra desde donde van los musicos
	const int numCanciones = 2;
	Sonido cancionesBanda[numCanciones] = { Sonido("Audio/banda_son_calenda.mp3"), Sonido("Audio/banda_dios_nunca_muere.mp3") };
	int cancionActual = 0;
	Sonido murmullo = Sonido("Audio/murmullo.flac");
	Sonido chispasTorito = Sonido("Audio/chispas_torito.flac");
	Sonido campanas = Sonido("Audio/campanas.mp3");
	Sonido cohete = Sonido("Audio/cohete.mp3");
	Sonido coheteTrueno = Sonido("Audio/cohete_trueno.mp3");
	if (hayAudio)
	{
		for (int i = 0; i < numCanciones; i++)
		{
			cancionesBanda[i].CargarSonido(&motorAudio, false, 12.0f);
		}
		//el murmullo de la gente se oye igual en toda la escena
		murmullo.CargarSonido(&motorAudio, true, 0.0f);
		murmullo.PonerVolumen(0.35f);
		chispasTorito.CargarSonido(&motorAudio, true, 3.0f);
		chispasTorito.PonerVolumen(0.5f);
		campanas.CargarSonido(&motorAudio, false, 60.0f);
		campanas.PonerVolumen(0.8f);
		//las campanas suenan desde las torres de Santo Domingo
		campanas.PonerPosicion(glm::vec3(60.0f, 22.0f, -139.5f));
		cohete.CargarSonido(&motorAudio, false, 40.0f);
		coheteTrueno.CargarSonido(&motorAudio, false, 40.0f);
		murmullo.Tocar();
	}

	//glm::perspective usa radianes, 45.0f da unos 58 grados
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// ================== 6. CICLO PRINCIPAL ==================
	//todo lo que esta dentro del while se repite cada frame hasta que se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		//el primer frame trae todo lo que tardo en cargar, se limita para que el desfile no brinque
		GLfloat segundos = glm::min(now - lastTime, 0.1f);
		lastTime = now;

		//eventos de teclado y mouse
		glfwPollEvents();

		//el mouse se lee siempre para que no se acumule con una camara fija
		GLfloat cambioX = mainWindow.getXChange();
		GLfloat cambioY = mainWindow.getYChange();
		//al soltar una camara fija, la libre copia la vista de camaraActual
		if (mainWindow.getsoltarCamara())
		{
			camera.tomarVistaDe(*camaraActual);
		}
		if (mainWindow.getcamaraActiva() == 1)
		{
			camaraActual = &camaraFachada;
		}
		else if (mainWindow.getcamaraActiva() == 2)
		{
			camaraActual = &camaraDesfile;
		}
		else if (mainWindow.getcamaraActiva() == 3)
		{
			camaraActual = &camaraZocalo;
		}
		else if (mainWindow.getcamaraActiva() == 4)
		{
			camaraActual = &camaraAerea;
		}
		else
		{
			camaraActual = &camera;
			camera.keyControl(mainWindow.getsKeys(), deltaTime);
			camera.mouseControl(cambioX, cambioY);
			//la libre no sale de la ciudad ni sube mas de 30 m, asi no se ve el borde de la escena
			camera.limitarPosicion(glm::vec3(-54.0f, 1.0f, -200.0f), glm::vec3(68.0f, 30.0f, 126.0f));
		}

		//el desfile avanza; frente al kiosco se queda quieto unos segundos y vuelve a empezar
		if (mainWindow.getreiniciarDesfile())
		{
			avanceDesfile = inicioDesfile;
			empiezaDesfile = true;
			//con la R la banda tambien vuelve a su primera cancion
			cancionesBanda[cancionActual].Pausar();
			cancionActual = 0;
			cancionesBanda[cancionActual].DesdeInicio();
		}
		if (!mainWindow.getdesfilePausado())
		{
			tiempoDesfile += segundos;
			avanceDesfile += velocidadDesfile * segundos;
			if (avanceDesfile > largoRecorrido + velocidadDesfile * esperaDesfile)
			{
				avanceDesfile = inicioDesfile;
				empiezaDesfile = true;
			}
			for (int i = 0; i < numChinas; i++)
			{
				animate(animChina[i], deltaTime);
			}
			animate(animMarmota, deltaTime);
			ActualizarDulces(segundos);
		}
		//en pausa tambien se dejan de mover las piernas
		bool desfileCaminando = avanceDesfile < largoRecorrido && !mainWindow.getdesfilePausado();

		//la camara del desfile va adelante y a la izquierda del primero, viendo hacia la mitad de la fila
		float distanciaPrimero = glm::min(avanceDesfile, largoRecorrido);
		camaraDesfile.mirarHacia(PosicionRecorrido(distanciaPrimero + 4.0f, 3.0f) + glm::vec3(0.0f, 3.2f, 0.0f),
			PosicionRecorrido(distanciaPrimero - 8.0f, 0.0f) + glm::vec3(0.0f, 1.0f, 0.0f));

		//---------- audio ----------
		if (hayAudio)
		{
			//el oido va en la camara activa, asi con cada camara se oye distinto
			glm::vec3 oido = camaraActual->getCameraPosition();
			glm::vec3 mira = camaraActual->getCameraDirection();
			ma_engine_listener_set_position(&motorAudio, 0, oido.x, oido.y, oido.z);
			ma_engine_listener_set_direction(&motorAudio, 0, mira.x, mira.y, mira.z);
			if (mainWindow.getsinSonido())
			{
				ma_engine_set_volume(&motorAudio, 0.0f);
			}
			else
			{
				ma_engine_set_volume(&motorAudio, 1.0f);
			}

			//al salir del atrio repican las campanas y truena un cohete
			if (empiezaDesfile)
			{
				campanas.DesdeInicio();
				cohete.PonerPosicion(PosicionRecorrido(0.0f, 0.0f) + glm::vec3(0.0f, 15.0f, 0.0f));
				cohete.DesdeInicio();
				empiezaDesfile = false;
				desfileLlego = false;
			}
			//al llegar frente al kiosco truena otro
			if (avanceDesfile >= largoRecorrido && !desfileLlego)
			{
				coheteTrueno.PonerPosicion(PosicionRecorrido(largoRecorrido, 0.0f) + glm::vec3(0.0f, 15.0f, 0.0f));
				coheteTrueno.DesdeInicio();
				desfileLlego = true;
			}

			//la musica sale del musico de en medio y las chispas del torito
			cancionesBanda[cancionActual].PonerPosicion(PosicionRecorrido(distanciaPrimero - atrasDesfile[6], 0.0f) + glm::vec3(0.0f, 1.5f, 0.0f));
			chispasTorito.PonerPosicion(PosicionRecorrido(distanciaPrimero - atrasDesfile[8], 0.0f) + glm::vec3(0.0f, 2.0f, 0.0f));
			//al acabar una cancion empieza la otra
			if (cancionesBanda[cancionActual].Termino())
			{
				cancionActual = (cancionActual + 1) % numCanciones;
				cancionesBanda[cancionActual].DesdeInicio();
			}
			//en pausa la banda deja de tocar y el torito de echar chispas
			if (mainWindow.getdesfilePausado())
			{
				cancionesBanda[cancionActual].Pausar();
				chispasTorito.Pausar();
			}
			else
			{
				cancionesBanda[cancionActual].Tocar();
				chispasTorito.Tocar();
			}
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//el skybox va primero
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

		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camaraActual->calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camaraActual->getCameraPosition().x, camaraActual->getCameraPosition().y, camaraActual->getCameraPosition().z);

		//con la tecla N cambia el sol y se encienden las farolas
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

		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		glm::mat4 model(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //blanco, la textura se ve con sus colores normales

		// ---------- objetos opacos ----------
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//los pisos ya estan en el mundo, con matriz identidad
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//el piso lejano, blanco con un color parecido al de los techos
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

		//Santo Domingo en el oriente del atrio, puerta en x 56
		//el modelo tiene la fachada hacia Z positivo, se gira -90 para verla hacia X negativo
		//los barandales son png, el discard del shader deja ver lo de atras
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(56.0f, altoBanqueta, -139.5f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		santoDomingo.RenderModel();

		//Sangre de Cristo, fachada hacia el oriente, al andador
		//se gira 90 grados para que la fachada vea hacia X positivo
		//la fachada queda en x -9, la orilla de los escalones en x -6
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-9.0f, altoBanqueta, -93.6f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		sangreDeCristo.RenderModel();

		//kiosco al centro del Zocalo
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, altoBanqueta, 77.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		kiosco.RenderModel();

		//jardineras, ya tienen sus coordenadas en el mundo
		model = glm::mat4(1.0);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		canteraTexture.UseTexture();
		meshListModel[8]->RenderMeshModel();
		tierraTexture.UseTexture();
		meshListModel[9]->RenderMeshModel();

		//agaves en las jardineras del atrio, 10 por 7 en cada una
		//cada uno gira distinto
		for (int i = 0; i < numJardinerasAtrio; i++)
		{
			const Rectangulo& j = jardineras[i];
			for (int columna = 0; columna < 10; columna++)
			{
				for (int fila = 0; fila < 7; fila++)
				{
					float x = j.x0 + (j.x1 - j.x0) * (columna + 0.5f) / 10.0f;
					float z = j.z0 + (j.z1 - j.z0) * (fila + 0.5f) / 7.0f;
					float giro = (columna * 2 + fila) * 45.0f;
					model = glm::mat4(1.0);
					model = glm::translate(model, glm::vec3(x, altoBanqueta + altoJardinera + 0.01f, z));
					model = glm::rotate(model, giro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
					agave.RenderModel();
				}
			}
		}

		//farolas, mismo modelo en cada lugar con su giro, en blanco para que se vea tal cual
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

		//desfile, cada personaje va detras del primero segun su lugar en la fila
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		int numChina = 0;
		for (int i = 0; i < numDesfile; i++)
		{
			float distancia = glm::min(avanceDesfile, largoRecorrido) - atrasDesfile[i];
			model = glm::mat4(1.0);
			model = glm::translate(model, PosicionRecorrido(distancia, ladoDesfile[i]));
			model = glm::rotate(model, GiroRecorrido(distancia) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			//el paso va con lo que avanza, asi los pies no se resbalan; cada uno empieza en otro momento
			float fase = distancia / zancada * 2.0f * 3.1416f + i * 1.3f;
			if (tipoDesfile[i] == MARMOTA)
			{
				DibujarDanzanteMarmota(model, fase, desfileCaminando, uniformModel);
			}
			else if (tipoDesfile[i] == CHINA)
			{
				DibujarChina(model, numChina, tiempoDesfile, uniformModel);
				numChina++;
			}
			else if (tipoDesfile[i] == MUSICO)
			{
				DibujarMusico(model, fase, desfileCaminando, tiempoDesfile + i * 0.7f, uniformModel);
			}
			else
			{
				DibujarTorito(model, fase, desfileCaminando, tiempoDesfile, uniformModel);
			}
		}

		for (int i = 0; i < numEspectadores; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posicionEspectadores[i]);
			model = glm::rotate(model, giroEspectadores[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			if (esMujer[i])
			{
				DibujarEspectadora(model, now + i * 0.9f, ropaEspectadores[i], uniformModel, uniformColor);
			}
			else
			{
				DibujarEspectador(model, now + i * 0.9f, ropaEspectadores[i], uniformModel);
			}
			if (i % 2 == 0)
			{
				DibujarGlobos(model, now + i * 0.9f, uniformModel, uniformColor);
			}
		}

		// ---------- objetos con transparencia ----------
		//van al final, para que se vea lo que esta detras
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//telones al final de las calles que cruzan
		telonTexture.UseTexture();
		for (int i = 0; i < numTelones; i++)
		{
			DibujarPlano(posicionTelones[i], giroTelones[i], 12.0f, 12.0f, color, uniformModel, uniformColor);
		}

		//una tira si y otra no va girada, para que los colores no queden en fila
		papelPicadoTexture.UseTexture();
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		for (int i = 0; i < numPapelPicado; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, altoBanqueta + altoPapelPicado, zPapelPicado[i]));
			if (i % 2 == 1)
			{
				model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshListModel[10]->RenderMeshModel();
		}

		//van aqui porque sus hojas tienen orillas transparentes
		//flamboyanes cada 9 m sobre la jardinera del norte, se agrandan un poco, son de unos 9 m
		//cada uno gira distinto
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(10.0f + i * 9.0f, ySobreJardinera, -176.75f));
			model = glm::rotate(model, (i * 70.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			flamboyan.RenderModel();
		}

		//palmeras en los macetones del sur del atrio
		for (int i = 0; i < 3; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(10.0f + i * 16.0f, ySobreJardinera, -110.0f));
			model = glm::rotate(model, (i * 120.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			palmera.RenderModel();
		}

		//arboles del jardin Labastida, jacaranda y ligeros turnandose
		for (int i = 0; i < numArbolesLabastida; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posicionArbolesLabastida[i]);
			model = glm::rotate(model, (i * 55.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i % 2 == 1)
			{
				jacaranda.RenderModel();
			}
			else if (i % 4 == 0)
			{
				arbolLigero1.RenderModel();
			}
			else
			{
				arbolLigero2.RenderModel();
			}
		}

		for (int i = 0; i < numArbolesAtrio; i++)
		{
			DibujarModelo(jacaranda, posicionArbolesAtrio[i], i * 100.0f, 1.0f, uniformModel);
		}

		//arboles del Zocalo, se agrandan un poco
		for (int i = 0; i < numArbolesZocalo; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posicionArbolesZocalo[i]);
			model = glm::rotate(model, (i * 55.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i % 2 == 0)
			{
				arbolZocalo1.RenderModel();
			}
			else
			{
				arbolZocalo2.RenderModel();
			}
		}

		for (int i = 0; i < numCacaloxochitl; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posicionCacaloxochitl[i]);
			model = glm::rotate(model, (i * 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			cacaloxochitl.RenderModel();
		}

		for (int i = 0; i < numEstantes; i++)
		{
			float giro = 90.0f;
			if (posicionEstantes[i].x > 0.0f)
			{
				giro = -90.0f;
			}
			DibujarEstanteMacetas(posicionEstantes[i], giro, i, uniformModel, uniformColor);
		}

		//flores sueltas, se agrandan un poco y cada una gira distinto
		for (int i = 0; i < numFlores; i++)
		{
			DibujarModelo(flores[i % numModelosFlor], posicionFlores[i], i * 47.0f, 2.4f, uniformModel);
		}

		//bancas, las segundas se giran hacia el norte
		for (int i = 0; i < numBancas; i++)
		{
			float giro = 0.0f;
			if (i >= numBancas / 2)
			{
				giro = 180.0f;
			}
			DibujarModelo(banca, posicionBancas[i], giro, 1.0f, uniformModel);
		}

		for (int i = 0; i < numCarritos; i++)
		{
			DibujarModelo(carritoElotes, posicionCarritos[i], giroCarritos[i], 1.0f, uniformModel);
		}

		for (int i = 0; i < numBugambilias; i++)
		{
			DibujarModelo(bugambilia, posicionBugambilias[i], i * 130.0f, 1.0f, uniformModel);
		}
		for (int i = 0; i < numBugambiliasMuro; i++)
		{
			DibujarModelo(bugambiliaMuro, posicionBugambiliasMuro[i], giroBugambiliasMuro[i], 1.0f, uniformModel);
		}

		//puesto de textiles, mesas en los pares y tendederos en los nones, todo girado -90 grados
		for (int i = 0; i < numPuestosTextiles; i++)
		{
			if (i % 2 == 0)
			{
				DibujarModelo(mesaToldo, glm::vec3(6.2f, altoBanqueta, zPuestosTextiles[i]), -90.0f, 1.0f, uniformModel);
			}
			else
			{
				DibujarModelo(tendedero, glm::vec3(5.6f, altoBanqueta, zPuestosTextiles[i]), -90.0f, 1.0f, uniformModel);
			}
		}

		//puestos de artesanias en la orilla norte del jardin Labastida, junto a los textiles
		//cada uno en un petate, con las piezas viendo hacia el sur
		float yPetate = altoBanqueta + 0.01f;
		//puesto de barro y pan
		DibujarModelo(petate, glm::vec3(10.0f, altoBanqueta, -73.5f), 180.0f, 1.0f, uniformModel);
		DibujarModelo(cazuela, glm::vec3(9.6f, yPetate, -73.1f), 0.0f, 1.0f, uniformModel);
		DibujarModelo(olla, glm::vec3(9.55f, yPetate, -73.5f), 0.0f, 1.0f, uniformModel);
		DibujarModelo(taza, glm::vec3(9.6f, yPetate, -73.85f), 0.0f, 1.0f, uniformModel);
		DibujarModelo(taza, glm::vec3(9.8f, yPetate, -74.05f), 30.0f, 1.0f, uniformModel);
		//los panes van apilados en la canasta, que tiene el fondo a 6 cm y la orilla a 28 cm,
		//para que los de arriba se asomen; el asa queda de norte a sur y no les estorba
		glm::vec3 canastaPan = glm::vec3(10.3f, yPetate, -73.4f);
		DibujarModelo(canasta, canastaPan, 90.0f, 1.0f, uniformModel);
		DibujarModelo(panYema, canastaPan + glm::vec3(-0.075f, 0.06f, 0.0f), 90.0f, 1.0f, uniformModel);
		DibujarModelo(panYema, canastaPan + glm::vec3(0.075f, 0.06f, 0.0f), 60.0f, 1.0f, uniformModel);
		DibujarModelo(panYema, canastaPan + glm::vec3(0.0f, 0.13f, -0.08f), 130.0f, 1.0f, uniformModel);
		DibujarModelo(panYema, canastaPan + glm::vec3(0.0f, 0.13f, 0.08f), 170.0f, 1.0f, uniformModel);
		DibujarModelo(panYema, canastaPan + glm::vec3(-0.09f, 0.21f, 0.0f), 100.0f, 1.0f, uniformModel);
		DibujarModelo(panYema, canastaPan + glm::vec3(0.09f, 0.21f, 0.0f), 150.0f, 1.0f, uniformModel);
		DibujarModelo(canasta, glm::vec3(9.1f, altoBanqueta, -74.7f), 90.0f, 1.0f, uniformModel);
		//puesto de alebrijes
		DibujarModelo(petate, glm::vec3(16.0f, altoBanqueta, -73.5f), 180.0f, 1.0f, uniformModel);
		DibujarModelo(alebrijeCoyote, glm::vec3(15.7f, yPetate, -73.1f), 0.0f, 1.0f, uniformModel);
		DibujarModelo(alebrijePajaro, glm::vec3(15.55f, yPetate, -73.4f), 0.0f, 1.0f, uniformModel);
		DibujarModelo(alebrijePerrito, glm::vec3(15.65f, yPetate, -73.8f), 0.0f, 1.0f, uniformModel);
		DibujarModelo(alebrijeCoyote, glm::vec3(16.4f, yPetate, -73.2f), -30.0f, 1.0f, uniformModel);
		DibujarModelo(alebrijePajaro, glm::vec3(16.4f, yPetate, -73.8f), 30.0f, 1.0f, uniformModel);
		DibujarModelo(alebrijePerrito, glm::vec3(16.45f, yPetate, -73.45f), -10.0f, 1.0f, uniformModel);
		DibujarModelo(canasta, glm::vec3(15.1f, altoBanqueta, -74.7f), 90.0f, 1.0f, uniformModel);

		//fuente al norte del kiosco, a un lado del pasillo del desfile
		//va aqui porque el agua es transparente
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(7.0f, altoBanqueta, 55.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		fuenteZocalo.RenderModel();

		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	//el audio se detiene antes de que se borren los sonidos al salir de main
	if (hayAudio)
	{
		ma_engine_stop(&motorAudio);
	}
	return 0;
}
