#include "Texture.h"
#include "CommonValues.h"

Texture::Texture()
{
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = 0;
}
Texture::Texture(const char *FileLoc)
{
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = FileLoc;
}

bool Texture::LoadTextureA()
{
	return LoadTextureFile(true);
}
bool Texture::LoadTexture()
{
	return LoadTextureFile(false);
}

//carga la imagen de fileLocation y crea la textura, regresa false si no encuentra el archivo
bool Texture::LoadTextureFile(bool useAlpha)
{
	//RGBA guarda transparencia, RGB no
	int canales = STBI_rgb;
	GLenum formato = GL_RGB;
	if (useAlpha)
	{
		canales = STBI_rgb_alpha;
		formato = GL_RGBA;
	}

	//las imagenes empiezan arriba a la izquierda y OpenGL abajo a la izquierda, por eso se voltean
	stbi_set_flip_vertically_on_load(true);
	//texData son los bytes de la imagen (los pixeles)
	unsigned char *texData = stbi_load(fileLocation, &width, &height, &bitDepth, canales);
	if (!texData)
	{
		printf("No se encontro la textura: %s\n", fileLocation);
		return false;
	}
	glGenTextures(1, &textureID); //parecido al VAO: se crea la textura y se le da un id
	glBindTexture(GL_TEXTURE_2D, textureID); //textura 2D, para superficies es suficiente

	//lo que pasa cuando las UV se salen del rango de 0 a 1
	//GL_REPEAT repite la imagen, GL_MIRRORED_REPEAT la repite en espejo, GL_CLAMP_TO_EDGE estira el borde
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //eje S, que es como X
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //eje T, que es como Y
	//MIN_FILTER se usa cuando la textura se ve mas chica, de lejos, y MAG_FILTER cuando se ve mas grande, de cerca
	//GL_LINEAR mezcla los texels cercanos (se ve suave), GL_NEAREST toma el mas cercano (se ve pixeleado)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, formato, width, height, 0, formato, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind
	stbi_image_free(texData); //ya se copio a la GPU, se libera la imagen
	return true;
}
void Texture::ClearTexture()
{
	glDeleteTextures(1, &textureID);
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = "";
}
void Texture::UseTexture()
{
	//texture unit 0, es la que lee el sampler theTexture del shader
	//si algun dia se usan varias texturas al mismo tiempo hay que usar GL_TEXTURE1, etc. y un uniform para cada una
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

}

Texture::~Texture()
{
	ClearTexture();
}
