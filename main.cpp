#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL.h>
#include <SDL_events.h>
#include <sstream>
#include <vector>
#include "color.h"
#include "framebuffer.h"
#include "uniform.h"
#include "shaders.h"
#include "fragment.h"
#include "triangle.h"
#include "camera.h"
#include "ObjLoader.h"
#include "noise.h"
#include "model.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Color currentColor;

const float MIN_ZOOM = 0.5f;
const float MAX_ZOOM = 1.0f;

std::vector<Model> models;

glm::vec3 cameraPosition(0.0f, 0.0f, 3.0f); // Inicializa la posición de la cámara
float zoom = 1.0f; // Factor de zoom inicial


bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error: Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Celestial Bodies Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error: Failed to create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error: Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    setupNoise();

    return true;
}

void setColor(const Color& color) {
    currentColor = color;
}


void render() {
    for (const auto& model : models) {
        // 1. Vertex Shader
        std::vector<Vertex> transformedVertices(model.vertices.size() / 3);
        for (size_t i = 0; i < model.vertices.size() / 3; ++i) {
            Vertex vertex = {model.vertices[3 * i], model.vertices[3 * i + 1], model.vertices[3 * i + 2]};
            transformedVertices[i] = vertexShader(vertex, model.uniforms);
        }

        // 2. Primitive Assembly
        std::vector<std::vector<Vertex>> assembledVertices(transformedVertices.size() / 3);
        for (size_t i = 0; i < transformedVertices.size() / 3; ++i) {
            Vertex edge1 = transformedVertices[3 * i];
            Vertex edge2 = transformedVertices[3 * i + 1];
            Vertex edge3 = transformedVertices[3 * i + 2];
            assembledVertices[i] = {edge1, edge2, edge3};
        }

        // 3. Rasterization
        std::vector<Fragment> fragments;

        for (size_t i = 0; i < assembledVertices.size(); ++i) {
            std::vector<Fragment> rasterizedTriangle = triangle(
                    assembledVertices[i][0],
                    assembledVertices[i][1],
                    assembledVertices[i][2]
            );
            fragments.insert(fragments.end(), rasterizedTriangle.begin(), rasterizedTriangle.end());
        }

        // 4. Fragment Shader
        for (size_t i = 0; i < fragments.size(); ++i) {
            Fragment (*fragmentShader)(Fragment &) = nullptr;

            switch (model.currentShader) {
                case ROCOSO:
                    fragmentShader = planetaRocoso;
                    break;
                case GASEOSO:
                    fragmentShader = giganteGaseoso;
                    break;
                case ESTRELLA:
                    fragmentShader = estrella;
                    break;
                case LUNA:
                    fragmentShader = Luna;
                    break;
                case VOLCANICO:
                    fragmentShader = planetaVolcanico;
                    break;
                case CRISTAL:
                    fragmentShader = planetaCristal;
                    break;
                case HIELO:
                    fragmentShader = planetaHielo;
                    break;
                default:
                    std::cerr << "Error: Shader no reconocido." << std::endl;
                    break;
            }
            const Fragment &fragment = fragmentShader(fragments[i]);

            point(fragment);
        }
    }
}

glm::mat4 createViewportMatrix(size_t screenWidth, size_t screenHeight) {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

int SDL_main(int argc, char* argv[]) {
    ShaderType Shader1 = ROCOSO;
    ShaderType Shader2 = GASEOSO;
    ShaderType Shader3 = ESTRELLA;
    ShaderType Shader4 = VOLCANICO;
    ShaderType Shader5 = CRISTAL;
    ShaderType Shader6 = HIELO;

    if (!init()) {
        return 1;
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> texCoords;
    std::vector<Face> faces;
    std::vector<glm::vec3> vertexBufferObject; // This will contain both vertices and normals

    loadOBJ("esfera.obj", vertices, normals, texCoords, faces);

    for (const auto& face : faces)
    {
        for (int i = 0; i < 3; ++i)
        {
            // Get the vertex position
            glm::vec3 vertexPosition = vertices[face.vertexIndices[i]];

            // Get the normal for the current vertex
            glm::vec3 vertexNormal = normals[face.normalIndices[i]];

            // Get the texture for the current vertex
            glm::vec3 vertexTexture = texCoords[face.texIndices[i]];

            // Add the vertex position and normal to the vertex array
            vertexBufferObject.push_back(vertexPosition);
            vertexBufferObject.push_back(vertexNormal);
            vertexBufferObject.push_back(vertexTexture);
        }
    }

    Uniform uniforms;

    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = glm::mat4(1);
    glm::mat4 projection = glm::mat4(1);

    glm::vec3 translationVector(0.0f, 0.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 0.0f, 1.0f); // Rotate around the Z-axis
    glm::vec3 scaleFactor(1.0f, 1.0f, 1.0f);

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), translationVector);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleFactor);

    // Initialize a Camera object
    Camera camera;
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 4.0f);
    camera.targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    // Projection matrix
    float fovInDegrees = 130.0f;
    float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT); // Assuming a screen resolution of 800x600
    float nearClip = 0.1f;
    float farClip = 100.0f;
    uniforms.projection = glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);

    // Viewport matrix
    uniforms.viewport = createViewportMatrix(SCREEN_WIDTH, SCREEN_HEIGHT);
    Uint32 frameStart, frameTime;
    std::string title = "FPS: ";
    int speed = 1.0f;

    // Definir velocidades de órbita para cada planeta
    float orbitSpeed1 = 1.0f;  // Velocidad del planeta más cercano a la estrella
    float orbitSpeed2 = 0.7f;
    float orbitSpeed3 = 0.5f;
    float orbitSpeed4 = 0.3f;
    float orbitSpeed5 = 0.2f;  // Velocidad del planeta más alejado de la estrella

    float orbitAngle1 = 0.0f; // Ángulo inicial de órbita para el planeta 1
    float orbitAngle2 = 0.0f; // Ángulo inicial de órbita para el planeta 2
    float orbitAngle3 = 0.0f; // Ángulo inicial de órbita para el planeta 3
    float orbitAngle4 = 0.0f; // Ángulo inicial de órbita para el planeta 4
    float orbitAngle5 = 0.0f; // Ángulo inicial de órbita para el planeta 5


    bool running = true;
    while (running) {
        frameStart = SDL_GetTicks();

        models.clear(); // Clear models vector at the beginning of the loop


        orbitAngle1 += orbitSpeed1;
        glm::mat4 rotation1 = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngle1), rotationAxis);

        orbitAngle2 += orbitSpeed2;
        glm::mat4 rotation2 = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngle2), rotationAxis);

        orbitAngle3 += orbitSpeed3;
        glm::mat4 rotation3 = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngle3), rotationAxis);

        orbitAngle4 += orbitSpeed4;
        glm::mat4 rotation4 = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngle4), rotationAxis);

        orbitAngle5 += orbitSpeed5;
        glm::mat4 rotation5 = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngle5), rotationAxis);


        Model Estrella;
        Estrella.modelMatrix = glm::mat4(1);
        Estrella.vertices = vertexBufferObject;
        Estrella.uniforms = uniforms;
        Estrella.currentShader = Shader3;
        models.push_back(Estrella); // Add planeta to models vector

        // Calcular las matrices de modelo para cada planeta
        uniforms.model = translation * rotation1 * scale;
        Model planeta;
        planeta.modelMatrix = glm::mat4(1);
        planeta.vertices = vertexBufferObject;
        planeta.uniforms = uniforms;
        planeta.currentShader = Shader1;
        planeta.uniforms.model = glm::translate(planeta.uniforms.model, glm::vec3(1.5f, 0.0f, 0.0f))
                                 * glm::scale(planeta.uniforms.model, glm::vec3(0.3f, 0.3f, 0.3f));
        models.push_back(planeta);

        uniforms.model = translation * rotation2 * scale;
        Model planeta2;
        planeta2.modelMatrix = glm::mat4(1);
        planeta2.vertices = vertexBufferObject;
        planeta2.uniforms = uniforms;
        planeta2.currentShader = Shader2;
        planeta2.uniforms.model = glm::translate(planeta2.uniforms.model, glm::vec3(2.5f, 0.0f, 0.0f))
                                  * glm::scale(planeta2.uniforms.model, glm::vec3(0.5f, 0.5f, 0.5f));
        models.push_back(planeta2);

        uniforms.model = translation * rotation3 * scale;
        Model planeta3;
        planeta3.modelMatrix = glm::mat4(1);
        planeta3.vertices = vertexBufferObject;
        planeta3.uniforms = uniforms;
        planeta3.currentShader = Shader4;
        planeta3.uniforms.model = glm::translate(planeta3.uniforms.model, glm::vec3(3.3f, 0.0f, 0.0f))
                                  * glm::scale(planeta3.uniforms.model, glm::vec3(0.4f, 0.4f, 0.4f));
        models.push_back(planeta3);

        uniforms.model = translation * rotation4 * scale;
        Model planeta4;
        planeta4.modelMatrix = glm::mat4(1);
        planeta4.vertices = vertexBufferObject;
        planeta4.uniforms = uniforms;
        planeta4.currentShader = Shader5;
        planeta4.uniforms.model = glm::translate(planeta4.uniforms.model, glm::vec3(4.1f, 0.0f, 0.0f))
                                  * glm::scale(planeta4.uniforms.model, glm::vec3(0.75f, 0.75f, 0.75f));
        models.push_back(planeta4);

        uniforms.model = translation * rotation5 * scale;
        Model planeta5;
        planeta5.modelMatrix = glm::mat4(1);
        planeta5.vertices = vertexBufferObject;
        planeta5.uniforms = uniforms;
        planeta5.currentShader = Shader6;
        planeta5.uniforms.model = glm::translate(planeta5.uniforms.model, glm::vec3(5.5f, 0.0f, 0.0f))
                                  * glm::scale(planeta5.uniforms.model, glm::vec3(0.5f, 0.5f, 0.5f));
        models.push_back(planeta5);


        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        // Mueve la cámara hacia la izquierda
                        camera.cameraPosition.x -= 1.0f;
                        break;
                    case SDLK_RIGHT:
                        // Mueve la cámara hacia la derecha
                        camera.cameraPosition.x += 1.0f;
                        break;
                    case SDLK_UP:
                        // Mueve la cámara hacia arriba
                        camera.cameraPosition.y += 1.0f;
                        break;
                    case SDLK_DOWN:
                        // Mueve la cámara hacia abajo
                        camera.cameraPosition.y -= 1.0f;
                        break;
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6: {
                        //hasta que no se presione otra tecla, que haga esto

                        int planetIndex = event.key.keysym.sym - SDLK_1;

                        // Verifica si el índice es válido
                        if (planetIndex >= 0 && planetIndex < models.size()) {

                            // Cambia el centro de la cámara hacia la posición del planeta seleccionado
                            camera.targetPosition = models[planetIndex].uniforms.model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

                            // Ajusta el zoom para acercar la cámara al planeta

                            zoom = 0.5f;


                        }
                        break;

                    }

                }


            }

            if ((event.type == SDL_KEYUP) && (event.key.keysym.sym == SDLK_2 || event.key.keysym.sym == SDLK_3 || event.key.keysym.sym == SDLK_4 || event.key.keysym.sym == SDLK_5 || event.key.keysym.sym == SDLK_6)) {
                camera.targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
                zoom = 1.0f;
            }



            if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y < 0) {
                    // Rueda del mouse hacia arriba (zoom in)
                    zoom *= 1.1f;
                } else if (event.wheel.y > 0) {
                    // Rueda del mouse hacia abajo (zoom out)
                    zoom /= 1.1f;
                }
                zoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, zoom));
            }
        }

        uniforms.view = glm::lookAt(
                camera.cameraPosition,
                camera.targetPosition,
                camera.upVector
        );

        // Ajusta la matriz de proyección para el zoom
        uniforms.projection = glm::perspective(glm::radians(fovInDegrees * zoom), aspectRatio, nearClip, farClip);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        clearFramebuffer();

        render();

        renderBuffer(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        // Calculate frames per second and update window title
        if (frameTime > 0) {
            std::ostringstream titleStream;
            titleStream << "FPS: " << 1000.0 / frameTime;  // Milliseconds to seconds
            SDL_SetWindowTitle(window, titleStream.str().c_str());
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


