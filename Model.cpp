#include"Model.h"

Model::Model(const char* file,
	/*Rotacion*/float radiansx, float radiansy, float radiansz,
	/*Traslacion*/float x, float y, float z,
	/*Escalacion*/float Escalarx, float Escalary, float Escalarz)
{
	// Make a JSON object
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	// Get the binary data
	Model::file = file;
	data = getData();

	// Matriz de rotación global
	glm::mat4 flipX = glm::rotate(glm::mat4(1.0f), glm::radians(radiansx), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(radiansz), glm::vec3(0.0f, 0.0f, 1.0f));	
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),glm::radians(radiansy),glm::vec3(0.0f, 1.0f, 0.0f));

	//glm::mat4 globalTransform = rotateZ * flipX * rotateY;
	
	glm::mat4 translateGlobal = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(Escalarx, Escalary, Escalarz));


	glm::mat4 globalTransform = translateGlobal * scaleMatrix * rotateZ * flipX * rotateY;



	// Busca el nodo raíz real desde scenes[0].nodes[0]
	unsigned int rootNode = 0;
	if (JSON.contains("scenes") && JSON["scenes"].size() > 0 && JSON["scenes"][0].contains("nodes") && JSON["scenes"][0]["nodes"].size() > 0)
		rootNode = JSON["scenes"][0]["nodes"][0];

	// Llama a traverseNode con la matriz globalTransform
	traverseNode(rootNode, globalTransform);
}



void Model::Draw(Shader& shader, Camera& camera)
{
	// Go over all meshes and draw each one
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i].Mesh::Draw(shader, camera, matricesMeshes[i]);
	}
}

/*

void Model::loadMesh(unsigned int indMesh)
{
	// Get all accessor indices
	unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
	unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];

	// Comprobar si TEXCOORD_0 existe y no es nulo
	unsigned int texAccInd = 0;
	bool hasTexCoords = false;
	if (JSON["meshes"][indMesh]["primitives"][0]["attributes"].contains("TEXCOORD_0") &&
		!JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"].is_null())
	{
		texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
		hasTexCoords = true;
	}

	unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

	// Use accessor indices to get all vertices components
	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
	std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
	std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);

	std::vector<float> texVec;
	std::vector<glm::vec2> texUVs;
	if (hasTexCoords)
	{
		texVec = getFloats(JSON["accessors"][texAccInd]);
		texUVs = groupFloatsVec2(texVec);
	}
	else
	{
		// Si no hay TEXCOORD_0, rellenar con ceros
		texUVs.resize(positions.size(), glm::vec2(0.0f, 0.0f));
	}

	// Combine all the vertex components and also get the indices and textures
	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
	std::vector<Texture> textures = getTextures();

	// Combine the vertices, indices, and textures into a mesh
	meshes.push_back(Mesh(vertices, indices, textures));
}
*/

void Model::loadMesh(unsigned int indMesh)
{
	// --------------------------
	// 1) Directorio del modelo
	// --------------------------
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	// --------------------------------------
	// 2) Leer posiciones, normales y UVs (si existen)
	// --------------------------------------
	unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
	unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
	bool hasTexCoords = false;
	unsigned int texAccInd = 0;
	if (JSON["meshes"][indMesh]["primitives"][0]["attributes"].contains("TEXCOORD_0") &&
		!JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"].is_null())
	{
		texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
		hasTexCoords = true;
	}

	unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

	// 2.1) Obtener vectores de floats y agruparlos
	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVec3(posVec);

	std::vector<float> normVec = getFloats(JSON["accessors"][normalAccInd]);
	std::vector<glm::vec3> normals = groupFloatsVec3(normVec);

	std::vector<glm::vec2> texUVs;
	if (hasTexCoords)
	{
		std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
		texUVs = groupFloatsVec2(texVec);
	}
	else
	{
		// Si no hay UVs, rellena con ceros (u=0,v=0)
		texUVs.resize(positions.size(), glm::vec2(0.0f, 0.0f));
	}

	// 2.2) Armar lista de vértices (Vertex = position, normal, color(fijo), texUV)
	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);

	// 2.3) Obtener índices
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);

	// ---------------------------------
	// 3) Extraer TODAS las texturas PBR
	// ---------------------------------
	std::vector<Texture> texturesParaEsteMesh;

	// ¿Esta primitiva tiene material asignado?
	if (JSON["meshes"][indMesh]["primitives"][0].contains("material"))
	{
		unsigned int matIndex = JSON["meshes"][indMesh]["primitives"][0]["material"];

		// 3.1) BASE COLOR (albedo)
		if (JSON["materials"][matIndex].contains("pbrMetallicRoughness")
			&& JSON["materials"][matIndex]["pbrMetallicRoughness"].contains("baseColorTexture"))
		{
			unsigned int texIndex = JSON["materials"][matIndex]
				["pbrMetallicRoughness"]["baseColorTexture"]["index"];
			unsigned int imgIndex = JSON["textures"][texIndex]["source"];
			std::string imagePath = JSON["images"][imgIndex]["uri"].get<std::string>();
			std::string fullPath = fileDirectory + imagePath;

			// Crea la textura con tipo "baseColor"
			Texture diffuseTex(fullPath.c_str(), "baseColor", texturesParaEsteMesh.size());
			texturesParaEsteMesh.push_back(diffuseTex);
		}

		// 3.2) METALLIC-ROUGHNESS
		if (JSON["materials"][matIndex].contains("pbrMetallicRoughness")
			&& JSON["materials"][matIndex]["pbrMetallicRoughness"].contains("metallicRoughnessTexture"))
		{
			unsigned int texIndex = JSON["materials"][matIndex]
				["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"];
			unsigned int imgIndex = JSON["textures"][texIndex]["source"];
			std::string imagePath = JSON["images"][imgIndex]["uri"].get<std::string>();
			std::string fullPath = fileDirectory + imagePath;

			Texture mrTex(fullPath.c_str(), "metallicRoughness", texturesParaEsteMesh.size());
			texturesParaEsteMesh.push_back(mrTex);
		}

		// 3.3) NORMAL MAP
		if (JSON["materials"][matIndex].contains("normalTexture"))
		{
			unsigned int texIndex = JSON["materials"][matIndex]["normalTexture"]["index"];
			unsigned int imgIndex = JSON["textures"][texIndex]["source"];
			std::string imagePath = JSON["images"][imgIndex]["uri"].get<std::string>();
			std::string fullPath = fileDirectory + imagePath;

			Texture normalTex(fullPath.c_str(), "normalMap", texturesParaEsteMesh.size());
			texturesParaEsteMesh.push_back(normalTex);
		}

		// 3.4) OCCLUSION MAP
		if (JSON["materials"][matIndex].contains("occlusionTexture"))
		{
			unsigned int texIndex = JSON["materials"][matIndex]["occlusionTexture"]["index"];
			unsigned int imgIndex = JSON["textures"][texIndex]["source"];
			std::string imagePath = JSON["images"][imgIndex]["uri"].get<std::string>();
			std::string fullPath = fileDirectory + imagePath;

			Texture occTex(fullPath.c_str(), "occlusionMap", texturesParaEsteMesh.size());
			texturesParaEsteMesh.push_back(occTex);
		}
	}

	// --------------------------------
	// 4) Finalmente, construimos el Mesh
	// --------------------------------
	meshes.push_back(Mesh(vertices, indices, texturesParaEsteMesh));
}


void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	// Nodo actual
	json node = JSON["nodes"][nextNode];

	// Obtener traslación, rotación y escala si existen
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	if (node.find("translation") != node.end())
	{
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
			transValues[i] = (node["translation"][i]);
		translation = glm::make_vec3(transValues);
	}
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end())
	{
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end())
	{
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}

	// Si el nodo tiene una matriz, úsala. Si no, compónla con TRS.
	glm::mat4 localMatrix = glm::mat4(1.0f);
	if (node.find("matrix") != node.end())
	{
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		localMatrix = glm::make_mat4(matValues);
	}
	else
	{
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 rot = glm::mat4_cast(rotation);
		glm::mat4 sca = glm::scale(glm::mat4(1.0f), scale);
		localMatrix = trans * rot * sca;
	}

	// Multiplica por la matriz acumulada del padre
	glm::mat4 matNextNode = matrix * localMatrix;

	// Si el nodo contiene una malla, cárgala
	if (node.find("mesh") != node.end())
	{
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}

	// Si el nodo tiene hijos, recursivamente procesa cada uno
	if (node.find("children") != node.end())
	{
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}
}

std::vector<unsigned char> Model::getData()
{
	// Create a place to store the raw text, and get the uri of the .bin file
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	// Store raw text data into bytesText
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	// Transform the raw text data into bytes and put them in a vector
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor)
{
	std::vector<float> floatVec;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Interpret the type and store it into numPerVert
	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	// Go over all the bytes in the data at the correct place using the properties from above
	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lengthOfData = count * 4 * numPerVert;
	for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i)
	{
		unsigned char bytes[] = { data[i++], data[i++], data[i++], data[i++] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		floatVec.push_back(value);
	}

	return floatVec;
}

std::vector<GLuint> Model::getIndices(json accessor)
{
	std::vector<GLuint> indices;
	// Validar que los campos existen y no son nulos
	if (!accessor.contains("componentType") || accessor["componentType"].is_null()) {
		throw std::runtime_error("Invalid or missing componentType in accessor.");
	}
	if (!accessor.contains("bufferView") || accessor["bufferView"].is_null()) {
		throw std::runtime_error("Invalid or missing bufferView in accessor.");
	}
	if (!accessor.contains("count") || accessor["count"].is_null()) {
		throw std::runtime_error("Invalid or missing count in accessor.");
	}

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	std::cout << bufferView.dump(4) << std::endl;

	if (!bufferView.contains("byteOffset") || bufferView["byteOffset"].is_null()) {
		throw std::runtime_error("Invalid or missing byteOffset in bufferView.");
	}
	unsigned int byteOffset = bufferView["byteOffset"];

	// Get indices with regards to their type: unsigned int, unsigned short, or short
	unsigned int beginningOfData = byteOffset + accByteOffset;
	if (componentType == 5125)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i)
		{
			unsigned char bytes[] = { data[i++], data[i++], data[i++], data[i++] };
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5123)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i)
		{
			unsigned char bytes[] = { data[i++], data[i++] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5122)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i)
		{
			unsigned char bytes[] = { data[i++], data[i++] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((GLuint)value);
		}
	}

	return indices;
}

std::vector<Texture> Model::getTextures()
{
	std::vector<Texture> textures;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	// Go over all images
	for (unsigned int i = 0; i < JSON["images"].size(); i++)
	{
		// uri of current texture
		std::string texPath = JSON["images"][i]["uri"];

		// Check if the texture has already been loaded
		bool skip = false;
		for (unsigned int j = 0; j < loadedTexName.size(); j++)
		{
			if (loadedTexName[j] == texPath)
			{
				textures.push_back(loadedTex[j]);
				skip = true;
				break;
			}
		}

		// If the texture has been loaded, skip this
		if (!skip)
		{
			// Load diffuse texture
			if (texPath.find("baseColor") != std::string::npos)
			{
				Texture diffuse = Texture((fileDirectory + texPath).c_str(), "diffuse", loadedTex.size());
				textures.push_back(diffuse);
				loadedTex.push_back(diffuse);
				loadedTexName.push_back(texPath);
			}
			// Load specular texture
			else if (texPath.find("metallicRoughness") != std::string::npos)
			{
				Texture specular = Texture((fileDirectory + texPath).c_str(), "specular", loadedTex.size());
				textures.push_back(specular);
				loadedTex.push_back(specular);
				loadedTexName.push_back(texPath);
			}
		}
	}

	return textures;
}

std::vector<Vertex> Model::assembleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i]
			}
		);
	}
	return vertices;
}

std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
{
	std::vector<glm::vec2> vectors;
	for (int i = 0; i < floatVec.size(); i)
	{
		vectors.push_back(glm::vec2(floatVec[i++], floatVec[i++]));
	}
	return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
{
	std::vector<glm::vec3> vectors;
	for (int i = 0; i < floatVec.size(); i)
	{
		vectors.push_back(glm::vec3(floatVec[i++], floatVec[i++], floatVec[i++]));
	}
	return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
{
	std::vector<glm::vec4> vectors;
	for (int i = 0; i < floatVec.size(); i++)
	{
		vectors.push_back(glm::vec4(floatVec[i++], floatVec[i++], floatVec[i++], floatVec[i++]));
	}
	return vectors;
}
