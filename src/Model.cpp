#include "Model.h"
#include <unordered_map>


Model::Model()
{
}

void Model::LoadModel(const std::string & fileName)
{
	Assimp::Importer importer;
	//Triangulate pasa los poligonos y quads a triangulos
	//GenSmoothNormals genera normales si el modelo no trae
	//JoinIdenticalVertices junta los vertices repetidos en uno solo
	const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	if (!scene)
	{
		printf("Fallo al cargar el modelo: %s (%s)\n", fileName.c_str(), importer.GetErrorString());
		return;
	}
	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
}

void Model::ClearModel()
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		if (MeshList[i])
		{
			delete MeshList[i];
			MeshList[i] = nullptr;
		}
	}

	for (unsigned int i = 0; i < TextureList.size(); i++)
	{
		Texture* tex = TextureList[i];
		if (tex)
		{
			//varios materiales pueden usar la misma textura, se quita de toda la lista para no borrarla dos veces
			for (unsigned int j = i; j < TextureList.size(); j++)
			{
				if (TextureList[j] == tex)
				{
					TextureList[j] = nullptr;
				}
			}
			delete tex;
		}
	}

	MeshList.clear();
	TextureList.clear();
	meshTotex.clear();
}

void Model::RenderModel()
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		//cada malla usa la textura de su material
		unsigned int materialIndex = meshTotex[i];
		if (materialIndex < TextureList.size() && TextureList[materialIndex])
		{
			TextureList[materialIndex]->UseTexture();
		}
		MeshList[i]->RenderMeshModel();
	}
}


Model::~Model()
{
}

//recorre el arbol de nodos del archivo y carga todas sus mallas
void Model::LoadNode(aiNode * node, const aiScene * scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

//pasa una malla de Assimp al formato de MeshModel, con posicion, coordenadas de textura y normal en cada vertice
void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
		//UV
		if (mesh->mTextureCoords[0]) //si el modelo trae coordenadas de textura
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
		else
		{
			vertices.insert(vertices.end(), { 0.0f, 0.0f });
		}
		//las normales se usan tal cual vienen del archivo, apuntando hacia afuera igual que en Blender
		vertices.insert(vertices.end(), { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	MeshModel* newMesh = new MeshModel();
	newMesh->CreateMeshModel(&vertices[0], &indices[0], vertices.size(), indices.size());
	MeshList.push_back(newMesh);
	meshTotex.push_back(mesh->mMaterialIndex);
}

//carga la textura difusa (map_Kd en el .mtl) de cada material
//si no tiene o no se encuentra, usa Textures/plain.png
void Model::LoadMaterials(const aiScene * scene)
{
	std::unordered_map<std::string, Texture*> loadedTextures; //para no cargar la misma textura dos veces
	TextureList.resize(scene->mNumMaterials);
	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];
		TextureList[i] = nullptr;
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				//se queda solo con el nombre del archivo y quita las carpetas, o sea todo lo que esta antes de la ultima diagonal
				//si no hay carpeta, idx + 1 vale 0 y se toma el nombre completo
				std::string fullPath = std::string(path.data);
				std::size_t idx = fullPath.find_last_of("\\/");
				std::string filename = fullPath.substr(idx + 1);

				std::string texPath = std::string("Textures/") + filename;
				//si ya se cargo esta textura antes, se reutiliza
				auto it = loadedTextures.find(texPath);
				if (it != loadedTextures.end())
				{
					TextureList[i] = it->second;
				}
				else
				{
					Texture* newTex = new Texture(texPath.c_str());
					std::string ext = filename.substr(filename.find_last_of('.') + 1);

					//tga y png pueden tener transparencia
					bool loaded = false;
					if (ext == "tga" || ext == "png")
					{
						loaded = newTex->LoadTextureA();
					}
					else
					{
						loaded = newTex->LoadTexture();
					}

					if (loaded)
					{
						TextureList[i] = newTex;
						loadedTextures[texPath] = newTex;
					}
					else
					{
						printf("Fallo al cargar la textura del modelo: %s\n", texPath.c_str());
						delete newTex;
					}
				}
			}
		}
		//si no tiene textura o no se pudo cargar, se le pone plain.png (blanca) para que no se vea negro
		if (!TextureList[i])
		{
			TextureList[i] = new Texture("Textures/plain.png");
			TextureList[i]->LoadTextureA();
		}
	}
}
