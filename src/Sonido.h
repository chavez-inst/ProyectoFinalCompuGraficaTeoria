#pragma once
#include <miniaudio.h>
#include <glm.hpp>

//un audio de la carpeta Audio que se toca con miniaudio
//si tiene radio, suena desde un lugar de la escena y se oye menos al alejarse
class Sonido
{
public:
	Sonido();
	Sonido(const char* FileLoc);
	bool CargarSonido(ma_engine* motor, bool enBucle, float radio);
	void Tocar();		//sigue desde donde se quedo
	void Pausar();
	void DesdeInicio();
	bool Termino();
	void PonerPosicion(glm::vec3 posicion);
	void PonerVolumen(float volumen);
	~Sonido();
private:
	ma_sound sonido;
	bool cargado;
	const char* fileLocation;
};
