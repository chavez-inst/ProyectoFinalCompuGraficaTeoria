#include <stdio.h>
#include "Sonido.h"

Sonido::Sonido()
{
	cargado = false;
	fileLocation = "";
}

Sonido::Sonido(const char* FileLoc)
{
	cargado = false;
	fileLocation = FileLoc;
}

//el radio es la distancia en metros a la que se oye completo; con 0 se oye igual en todos lados
bool Sonido::CargarSonido(ma_engine* motor, bool enBucle, float radio)
{
	//se lee poco a poco mientras suena, asi las canciones largas no se cargan completas en memoria
	ma_uint32 opciones = MA_SOUND_FLAG_STREAM;
	if (radio <= 0.0f)
	{
		opciones = opciones | MA_SOUND_FLAG_NO_SPATIALIZATION;
	}
	if (ma_sound_init_from_file(motor, fileLocation, opciones, NULL, NULL, &sonido) != MA_SUCCESS)
	{
		printf("No se pudo cargar el sonido %s\n", fileLocation);
		return false;
	}
	cargado = true;
	ma_sound_set_looping(&sonido, enBucle);
	if (radio > 0.0f)
	{
		ma_sound_set_min_distance(&sonido, radio);
	}
	return true;
}

void Sonido::Tocar()
{
	if (cargado)
	{
		ma_sound_start(&sonido);
	}
}

//se detiene pero recuerda donde iba
void Sonido::Pausar()
{
	if (cargado)
	{
		ma_sound_stop(&sonido);
	}
}

void Sonido::DesdeInicio()
{
	if (cargado)
	{
		ma_sound_seek_to_pcm_frame(&sonido, 0);
		ma_sound_start(&sonido);
	}
}

//true cuando un sonido que no va en bucle llega a su final
bool Sonido::Termino()
{
	if (cargado)
	{
		return ma_sound_at_end(&sonido);
	}
	return false;
}

void Sonido::PonerPosicion(glm::vec3 posicion)
{
	if (cargado)
	{
		ma_sound_set_position(&sonido, posicion.x, posicion.y, posicion.z);
	}
}

//1 es el volumen original del archivo
void Sonido::PonerVolumen(float volumen)
{
	if (cargado)
	{
		ma_sound_set_volume(&sonido, volumen);
	}
}

Sonido::~Sonido()
{
	if (cargado)
	{
		ma_sound_uninit(&sonido);
	}
}
