#pragma once
#include <glew.h>

//malla con textura y normales, 8 valores por vertice
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
