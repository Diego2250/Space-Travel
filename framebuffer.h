#pragma once
#include <array>
#include <algorithm>
#include "glm/glm.hpp"
#include <limits>
#include <mutex>
#include <SDL_render.h>
#include "color.h"  // Include your Color class header
#include "fragment.h"

constexpr size_t SCREEN_WIDTH = 1000;
constexpr size_t SCREEN_HEIGHT = 800;

FragColor blank{
        Color{0, 0, 0},
        std::numeric_limits<double>::max()
};

std::array<FragColor, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer;

// Create a 2D array of mutexes
std::array<std::mutex, SCREEN_WIDTH * SCREEN_HEIGHT> mutexes;

void point(Fragment f) {
    std::lock_guard<std::mutex> lock(mutexes[f.y * SCREEN_WIDTH + f.x]);

    if (f.y > 0 && f.x > 0 && f.y < SCREEN_HEIGHT && f.x < SCREEN_WIDTH && f.z < framebuffer[f.y * SCREEN_WIDTH + f.x].z) {
        framebuffer[f.y * SCREEN_WIDTH + f.x] = FragColor{f.color, f.z};
    }
}

// Función para generar posiciones de estrellas
std::vector<glm::vec2> generateStarPositions() {
    std::vector<glm::vec2> starPositions;

    // Genera un número fijo de estrellas en posiciones aleatorias
    const int numStars = 1000;
    for (int i = 0; i < numStars; ++i) {
        int x = std::rand() % SCREEN_WIDTH;
        int y = std::rand() % SCREEN_HEIGHT;
        starPositions.emplace_back(x, y);
    }

    return starPositions;
}

// Variable global para almacenar las posiciones de las estrellas
std::vector<glm::vec2> starPositions = generateStarPositions();

void clearFramebuffer() {
    std::fill(framebuffer.begin(), framebuffer.end(), blank);

    // Dibuja estrellas en el framebuffer
    for (const auto& position : starPositions) {
        size_t index = static_cast<size_t>(position.y) * SCREEN_WIDTH + static_cast<size_t>(position.x);
        framebuffer[index].color = Color(1.0f, 1.0f, 1.0f);
    }
}

void renderBuffer(SDL_Renderer* renderer) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    void* texturePixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &texturePixels, &pitch);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    Uint32 format = SDL_PIXELFORMAT_ARGB8888;
    SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);

    Uint32* texturePixels32 = static_cast<Uint32*>(texturePixels);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int framebufferY = SCREEN_HEIGHT - y - 1;  // Reverse the order of rows
            int index = y * (pitch / sizeof(Uint32)) + x;
            const Color& color = framebuffer[framebufferY * SCREEN_WIDTH + x].color;
            texturePixels32[index] = SDL_MapRGBA(mappingFormat, color.r, color.g, color.b, color.a);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_Rect textureRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, texture, NULL, &textureRect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}