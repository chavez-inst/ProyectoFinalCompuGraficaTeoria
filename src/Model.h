#pragma once
#include <vector>
#include<string>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "Mesh_tn.h"
#include "Texture.h"

//modelo cargado con Assimp; las texturas se buscan en Textures/
class Model
{
public:
	Model();

	void LoadModel(const std::string& fileName);
	void RenderModel();
	void RenderModel(Texture& textura); //todas las mallas con otra textura, para la misma pieza con otra ropa
	void ClearModel(); //despues de esto ya no se puede dibujar

	~Model();

private:
	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);
	std::vector<MeshModel*>MeshList;
	std::vector<Texture*>TextureList;
	std::vector<unsigned int>meshTotex; //textura que usa cada malla
};
