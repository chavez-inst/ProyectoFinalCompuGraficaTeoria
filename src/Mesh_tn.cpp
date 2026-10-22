#include "Mesh_tn.h"

MeshModel::MeshModel()
{
	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;
}

//ojo, numOfVertices es cuantos floats tiene el arreglo, o sea el numero de vertices por 8, y no cuantos vertices son
void MeshModel::CreateMeshModel(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	indexCount = numOfIndices;

	glGenVertexArrays(1, &VAO); //crear 1 VAO
	glBindVertexArray(VAO);

	//indices
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW);

	//vertices, GL_STATIC_DRAW porque no se van a modificar despues
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);

	//cada glVertexAttribPointer le dice al shader donde esta cada dato dentro de los 8 valores
	//en location 0 va la posicion, que son 3 valores desde la posicion 0 del vertice
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);
	//en location 1 van las coordenadas de textura S y T, que son 2 valores desde la posicion 3
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 3));
	glEnableVertexAttribArray(1);
	//en location 2 va la normal, que son 3 valores desde la posicion 5
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 5));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//dibuja la malla usando los indices
void MeshModel::RenderMeshModel()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//borra los buffers de la GPU para no ocupar memoria
void MeshModel::ClearMeshModel()
{
	if (IBO != 0)
	{
		glDeleteBuffers(1, &IBO);
		IBO = 0;
	}

	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	indexCount = 0;
}

MeshModel::~MeshModel()
{
	ClearMeshModel();
}
