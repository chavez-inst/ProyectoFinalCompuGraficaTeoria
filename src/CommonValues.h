#ifndef COMMONVALS
#define COMMONVALS
#include "stb_image.h"

//maximo de luces de cada tipo
//si se cambian aqui, tambien hay que cambiarlos en shaders/shader_light.frag (tienen que ser iguales)
const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 4;
#endif
