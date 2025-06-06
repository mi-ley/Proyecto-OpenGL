/*#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Normals (not necessarily normalized)
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;
// Texture Coordinates
layout (location = 3) in vec2 aTex;


// Outputs the current position for the Fragment Shader
out vec3 crntPos;
// Outputs the normal for the Fragment Shader
out vec3 Normal;
// Outputs the color for the Fragment Shader
out vec3 color;
// Outputs the texture coordinates to the Fragment Shader
out vec2 texCoord;



// Imports the camera matrix
uniform mat4 camMatrix;
// Imports the transformation matrices
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;


void main()
{
	// calculates current position
	crntPos = vec3(model * translation * -rotation * scale * vec4(aPos, 1.0f));
	// Assigns the normal from the Vertex Data to "Normal"
	Normal = aNormal;
	// Assigns the colors from the Vertex Data to "color"
	color = aColor;
	// Assigns the texture coordinates from the Vertex Data to "texCoord"
	texCoord = mat2(0.0, -1.0, 1.0, 0.0) * aTex;
	
	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * vec4(crntPos, 1.0);
}*/
#version 330 core

// -----------------------------------
// Atributos de entrada (layout = location X)
// -----------------------------------
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;       // (solo si tu glTF trae vertex colors; si no, lo ignoras)
layout (location = 3) in vec2 aTexCoord;

// -----------------------------------
// Variables de salida para el fragment
// -----------------------------------
out vec3 FragPos;   // posición en espacio mundo, para iluminación
out vec3 Normal;    // normal transformada al espacio mundo
out vec3 Color;     // color por vértice (opcional)
out vec2 TexCoord;  // UV sin modificar

// -----------------------------------
// Uniforms para matrices
// -----------------------------------
uniform mat4 model;      // MATRIZ model = (matrizGlobal * T * R * S)
uniform mat4 camMatrix;  // projection * view

void main()
{
    // 1) Transformar la posición al espacio mundo (model * aPos)
    FragPos = vec3(model * vec4(aPos, 1.0));

    // 2) Transformar la normal con la inversa transpuesta de 'model'
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // 3) Pasar color si existe (o ignorar)
    Color = aColor;

    // 4) Pasar coordenadas de textura tal cual vienen
    TexCoord = aTexCoord;

    // 5) Obtener gl_Position en Clip Space
    gl_Position = camMatrix * vec4(FragPos, 1.0);
}
