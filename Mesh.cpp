#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	VAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(indices);
	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}


void Mesh::Draw(Shader& shader,
	Camera& camera,
	glm::mat4 matrix,
	glm::vec3 translation,
	glm::quat rotation,
	glm::vec3 scale)
{
	// 1) Activa el shader y enlaza el VAO
	shader.Activate();
	VAO.Bind();

	// 2) Variables para contar cada tipo de textura PBR
	unsigned int countBaseColor = 0;
	unsigned int countMR = 0;
	unsigned int countNormal = 0;
	unsigned int countOcclusion = 0;

	// 3) Recorremos todas las texturas del mesh y las enlazamos
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;

		if (type == "baseColor")
		{
			num = std::to_string(countBaseColor++);
		}
		else if (type == "metallicRoughness")
		{
			num = std::to_string(countMR++);
		}
		else if (type == "normalMap")
		{
			num = std::to_string(countNormal++);
		}
		else if (type == "occlusionMap")
		{
			num = std::to_string(countOcclusion++);
		}
		// Construimos el nombre del uniform EXACTO que busca el fragment shader
		std::string uniformName = type + num;      // p.ej. "baseColor0", "metallicRoughness0", "normalMap0", "occlusionMap0"

		textures[i].texUnit(shader, uniformName.c_str(), i);
		textures[i].Bind();
	}

	// 4) Enviar posición de cámara (para cálculos especulares y demás)
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"),
		camera.Position.x, camera.Position.y, camera.Position.z);

	// 5) Enviar la matriz de vista+proyección (tu método Camera::Matrix)
	camera.Matrix(shader, "camMatrix");

	// 6) Construir y enviar la matriz 'model' combinada (matrix global + T * R * S)
	glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 R = glm::mat4_cast(rotation);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 modelMatrix = matrix * T * R * S;

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"),
		1, GL_FALSE, glm::value_ptr(modelMatrix));

	// 7) Finalmente dibujamos
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	// 8) (Opcional) Unbind VAO
	VAO.Unbind();
}
