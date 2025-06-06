#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;      // (opcional, si no tienes vertex colors lo ignoras)
in vec2 TexCoord;

// -----------------------------------
// Uniforms de iluminación
// -----------------------------------
uniform vec3 lightPos;
uniform vec4 lightColor;
uniform vec3 camPos;

// -----------------------------------
// Samplers para cada canal PBR
// -----------------------------------
uniform sampler2D baseColor0;          // canal Albedo / Base Color
uniform sampler2D metallicRoughness0;  // canal Metal & Roughness empacado en R y G
uniform sampler2D normalMap0;          // mapa de normales (RGB->[-1,1])
uniform sampler2D occlusionMap0;       // mapa de oclusión (canal rojo, [0,1])

out vec4 FragColor;

void main()
{
    // -------------------------------------------------------
    // 1) Albedo (base color)
    // -------------------------------------------------------
    vec3 albedo = texture(baseColor0, TexCoord).rgb;
    // Si tus UV vienen invertidas en Y, podrías hacer:
    // vec3 albedo = texture(baseColor0, vec2(TexCoord.x, 1.0 - TexCoord.y)).rgb;

    // -------------------------------------------------------
    // 2) Metallic & Roughness
    //    (glTF PBR estándar empaca metallic en .r y roughness en .g)
    // -------------------------------------------------------
    vec2 mr = texture(metallicRoughness0, TexCoord).rg;
    float metallic  = mr.r;
    float roughness = mr.g;

    // -------------------------------------------------------
    // 3) Normal mapping (simplificado: si no tienes tangentes, usa la normal directa)
    // -------------------------------------------------------
    vec3 tangentNormal = texture(normalMap0, TexCoord).xyz * 2.0 - 1.0; 
    // Aquí normalmente necesitaríamos la matriz TBN para llevar tangentNormal al espacio mundo.
    // Si tu glTF no proporcionó tangentes, puedes, en su lugar, usar:
    vec3 N = normalize(Normal);
    // O, si planeas soportar normalMap correctamente, tendrías que calcular TBN en CPU.
    vec3 finalNormal = N;  // (por simplicidad, ignoramos tangentNormal)

    // -------------------------------------------------------
    // 4) Oclusión ambiental
    // -------------------------------------------------------
    float ao = texture(occlusionMap0, TexCoord).r; // valor en [0,1]

    // -------------------------------------------------------
    // 5) Cálculo de iluminación (Blinn?Phong sencillo + oclusión)
    // -------------------------------------------------------
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir  = normalize(camPos - FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // Difusa
    float diff = max(dot(finalNormal, lightDir), 0.0);

    // Especular (Blinn?Phong)
    float spec = pow(max(dot(finalNormal, halfway), 0.0), 32.0);

    // Ambiente (factor 0.1) atenuado por AO
    vec3 ambient = 0.1 * albedo * ao;

    // Difuso
    vec3 diffuse = diff * albedo * lightColor.rgb;

    // Especular
    vec3 specularColor = vec3(1.0); 
    vec3 specular = spec * specularColor * lightColor.rgb;

    // -------------------------------------------------------
    // 6) Componer color final
    // -------------------------------------------------------
    vec3 colorFinal = ambient + diffuse + specular;
    FragColor = vec4(colorFinal, 1.0);
}
