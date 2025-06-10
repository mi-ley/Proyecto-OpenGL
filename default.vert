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
