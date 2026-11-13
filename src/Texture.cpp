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

//regresa false si no encuentra el archivo
bool Texture::LoadTextureFile(bool useAlpha)
{
	int canales = STBI_rgb;
	GLenum formato = GL_RGB;
	if (useAlpha)
	{
		canales = STBI_rgb_alpha;
		formato = GL_RGBA;
	}

	//las imagenes empiezan arriba a la izquierda y OpenGL abajo, por eso se voltean
	stbi_set_flip_vertically_on_load(true);
	unsigned char *texData = stbi_load(fileLocation, &width, &height, &bitDepth, canales);
	if (!texData)
	{
		printf("No se encontro la textura: %s\n", fileLocation);
		return false;
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//GL_REPEAT repite la imagen cuando las UV salen de 0 a 1
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//los mipmaps evitan que las texturas finas, como las losas, titilen al moverse de lejos
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, formato, width, height, 0, formato, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(texData); //ya esta en la GPU
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
	//unit 0, el que lee el sampler theTexture del shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

}

Texture::~Texture()
{
	ClearTexture();
}
