#pragma once
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "uniform.h"
#include "fragment.h"
#include "noise.h"
#include "print.h"

Vertex vertexShader(const Vertex& vertex, const Uniform& uniforms) {
    // Apply transformations to the input vertex using the matrices from the uniforms
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

    // Perspective divide
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    // Apply the viewport transform
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);

    // Transform the normal
    glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
    transformedNormal = glm::normalize(transformedNormal);

    glm::vec3 transformedWorldPosition = glm::vec3(uniforms.model * glm::vec4(vertex.position, 1.0f));

    // Return the transformed vertex as a vec3
    return Vertex{
            glm::vec3(screenVertex),
            transformedNormal,
            vertex.tex,
            transformedWorldPosition,
            vertex.position
    };
}

// Shader para el planeta rocoso
Fragment planetaRocoso(Fragment& fragment) {
    Color color;

    // Define colores base para las rocas y la superficie
    glm::vec3 surfaceColor = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 rockColor = glm::vec3(0.6f, 0.6f, 0.6f);
    glm::vec3 rocosaColor = glm::vec3(0.2f, 0.2f, 0.2f); // Color de la textura rocosa

    // Obtén las coordenadas UV
    glm::vec2 uv = glm::vec2(fragment.originalPos.x, fragment.originalPos.y);

    // Genera ruido para simular la textura del planeta
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    // Offset y escala para generar variaciones en las caras del planeta
    float offsetX = uv.x * 200.0f;
    float offsetY = uv.y * 200.0f;
    float scale = 100.0f;

    // Genera valores de ruido para las caras del planeta
    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.2f) * 0.7f; // Mapear [-1, 1] a [0, 1]

    // Combina los colores base con el ruido
    glm::vec3 surfaceFinalColor = glm::mix(surfaceColor, rockColor, noiseValue); // Color de la superficie
    glm::vec3 rocosaFinalColor = glm::mix(rockColor, rocosaColor, noiseValue); // Textura rocosa

    // Interpola entre la superficie y la textura rocosa
    glm::vec3 finalColor = glm::mix(surfaceFinalColor, rocosaFinalColor, noiseValue);

    color = Color(finalColor.x, finalColor.y, finalColor.z);

    fragment.color = color * fragment.intensity;

    return fragment;
}






// Shader para el planeta gaseoso
Fragment giganteGaseoso(Fragment& fragment) {
    Color color;

    // Obtener las coordenadas UV
    glm::vec2 uv = glm::vec2(fragment.originalPos.x * 2.0 - 1.0, fragment.originalPos.y * 2.0 - 1.0);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float offsetX = 1000.0f;
    float offsetY = 1000.0f;
    float scale = 900.0f;

    // Generar el valor de ruido para simular el movimiento del gas
    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.2f) * 0.9f; // Mapear [-1, 1] a [0, 1]

    // Generar colores diferentes para las "nubes de gas" en función de la posición y el ruido
    glm::vec3 cloudColor = glm::vec3(0.5 + uv.x + noiseValue, 0.5 + uv.y + noiseValue, 0.5 - uv.x + noiseValue);


    color = Color(cloudColor.x, cloudColor.y, cloudColor.z);

    fragment.color = color * fragment.intensity;

    return fragment;
}

// Shader para la estrella
Fragment estrella(Fragment& fragment) {
    Color color;

    // Genera colores aleatorios para la estrella
    float r = static_cast<float>(rand()) / RAND_MAX;
    float g = static_cast<float>(rand()) / RAND_MAX;
    float b = static_cast<float>(rand()) / RAND_MAX;

    // Añade un efecto de destello aleatorio
    float intensity = 1.0 + (rand() % 5) / 10.0;

    // Simula cambios de color sutiles con el tiempo
    float time = static_cast<float>(rand()) / RAND_MAX;
    r += sin(time);
    g += cos(time);
    b += sin(time * 0.5);

    // Añade un efecto de parpadeo aleatorio
    intensity *= 0.8 + (rand() % 4) / 10.0;

    // Añade un efecto de brillo aleatorio
    intensity *= 1.0 + (rand() % 2) / 10.0;

    // Limita los valores de color y intensidad
    r = std::min(1.0f, r);
    g = std::min(1.0f, g);
    b = std::min(1.0f, b);
    intensity = std::max(1.0f, intensity);

    // Asigna el color y la intensidad a la fragment
    color = Color(r, g, b);

    // Aumenta la intensidad para simular el brillo de la estrella
    fragment.color = color * intensity;

    return fragment;
}

Fragment Luna(Fragment& fragment) {
    Color color;

    // Definir los colores del planeta
    glm::vec3 rockColor = glm::vec3(0.7f, 0.7f, 0.7f);
    glm::vec3 surfaceColor = glm::vec3(0.4f, 0.4f, 0.4f);

    // Obtener las coordenadas UV
    glm::vec2 uv = glm::vec2(fragment.originalPos.x, fragment.originalPos.y);

    // Generar ruido para simular la textura del planeta
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float offsetX = 5000.0f;
    float offsetY = 8000.0f;
    float scale = 500.0f;

    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.0f) * 0.5f; // Mapear [-1, 1] a [0, 1]

    // Combinar los colores del planeta con el ruido
    glm::vec3 finalColor = glm::mix(rockColor, surfaceColor, noiseValue);

    color = Color(finalColor.x, finalColor.y, finalColor.z);

    fragment.color = color * fragment.intensity;

    return fragment;
}

Fragment planetaVolcanico(Fragment& fragment) {
    Color color;

    // Definir los colores del planeta volcánico
    glm::vec3 lavaColor = glm::vec3(1.0f, 0.1f, 0.0f);
    glm::vec3 rockColor = glm::vec3(0.6f, 0.6f, 0.6f);

    // Obtener las coordenadas UV
    glm::vec2 uv = glm::vec2(fragment.originalPos.x, fragment.originalPos.y);

    // Generar ruido para simular la textura del planeta
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float offsetX = 5000.0f;
    float offsetY = 8000.0f;
    float scale = 500.0f;

    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.0f) * 0.5f; // Mapear [-1, 1] a [0, 1]

    // Simular explosiones en la lava mediante variaciones de intensidad
    float intensity = 1.0f + noiseValue * 0.2f; // Añadir variación de intensidad

    // Combina los colores de lava y roca con el ruido y la intensidad
    glm::vec3 finalColor = glm::mix(rockColor, lavaColor, noiseValue);

    color = Color(finalColor.x, finalColor.y, finalColor.z);

    fragment.color = color * intensity;

    return fragment;
}


Fragment planetaCristal(Fragment& fragment) {
    Color color;

    // Definir los colores del planeta de cristal
    glm::vec3 crystalColor = glm::vec3(0.0f, 0.5f, 1.0f);

    // Obtener las coordenadas UV
    glm::vec2 uv = glm::vec2(fragment.originalPos.x, fragment.originalPos.y);

    // Generar ruido fractal para simular la textura del planeta
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
    noiseGenerator.SetFractalOctaves(6);
    noiseGenerator.SetFractalLacunarity(2.0f);
    noiseGenerator.SetFractalGain(0.5f);

    float offsetX = 5000.0f;
    float offsetY = 8000.0f;
    float scale = 500.0f;

    // Generar valores de ruido para la textura del planeta
    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.0f) * 0.5f; // Mapear [-1, 1] a [0, 1]

    // Combina el color de cristal con el ruido
    glm::vec3 finalColor = crystalColor * noiseValue;

    color = Color(finalColor.x, finalColor.y, finalColor.z);

    fragment.color = color * fragment.intensity;

    return fragment;
}


Fragment planetaHielo(Fragment& fragment) {
    Color color;

    // Definir el color base del planeta de hielo (celeste)
    glm::vec3 baseColor = glm::vec3(0.7f, 0.9f, 1.0f);

    // Obtener las coordenadas UV
    glm::vec2 uv = glm::vec2(fragment.originalPos.x, fragment.originalPos.y);

    // Generar ruido para simular la textura del planeta
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float offsetX = 5000.0f;
    float offsetY = 8000.0f;
    float scale = 500.0f;

    // Generar valores de ruido para la textura del planeta
    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.0f) * 0.5f; // Mapear [-1, 1] a [0, 1]

    // Combina el color base con el ruido
    glm::vec3 finalColor = baseColor * noiseValue;

    // Añadir elementos que se mueven utilizando el tiempo
    float time = SDL_GetTicks() / 1000.0f; // Obtener el tiempo en segundos

    // Generar elementos que se mueven más rápido
    glm::vec3 movingElements = glm::vec3(
            glm::sin(time * 2.0f) * 0.1f,
            glm::cos(time * 1.4f) * 0.1f,
            glm::sin(time * 1.0f) * 0.1f
    );

    finalColor += movingElements;

    color = Color(finalColor.x, finalColor.y, finalColor.z);

    fragment.color = color * fragment.intensity;

    return fragment;
}









