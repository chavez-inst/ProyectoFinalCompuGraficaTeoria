#pragma once
#include <glew.h>

//malla con textura y normales: cada vertice lleva 8 valores (x y z, s t, nx ny nz)
//la usan las figuras hechas a mano en main.cpp, los modelos cargados y el skybox
class MeshModel
{
public:
	MeshModel();

	void CreateMeshModel(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMeshModel();
	void ClearMeshModel();

	~MeshModel();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};
