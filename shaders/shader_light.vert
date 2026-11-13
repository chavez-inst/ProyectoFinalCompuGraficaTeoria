#version 330

//deben coincidir con los glVertexAttribPointer de Mesh_tn.cpp
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 vColor;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 color;


void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	vColor=vec4(color,1.0f);
	TexCoord = tex;
	//inversa transpuesta para que la normal no se deforme con escalas no uniformes
	Normal = mat3(transpose(inverse(model))) * norm;

	FragPos = (model * vec4(pos, 1.0)).xyz; //posicion del fragmento en el mundo
}
