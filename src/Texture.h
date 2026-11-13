#pragma once
#include<glew.h>

//textura 2D que se carga desde una imagen con stb_image
class Texture
{
public:
	Texture();
	Texture(const char* FileLoc);
	bool LoadTexture();  //sin transparencia, como .jpg
	bool LoadTextureA(); //con canal alfa, como .png o .tga
	void UseTexture();
	void ClearTexture();
	~Texture();
private:
	bool LoadTextureFile(bool useAlpha);
	GLuint textureID;
	int width, height, bitDepth;
	const char *fileLocation;

};
