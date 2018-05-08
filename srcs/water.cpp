#include <vector>
#include <SDL/SDL.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <omp.h>
#include <chrono>

const unsigned short    width       = 800;
const unsigned short    height      = 800;
const float             damping     = 0.9999999;
const unsigned short    nbAgents    = 15;

SDL_Surface *initGraphics(unsigned short width, unsigned short height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw (SDL_GetError());
    SDL_Surface *screen;

    screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE);

    if (screen == NULL)
        throw (SDL_GetError());
    
    SDL_WM_SetCaption("2D WATER RIPPLES", NULL);
    return screen;
}

void setPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    *((Uint32*)(surface->pixels)+x+(y*surface->w)) = color;
}

float newDirection(unsigned int position, unsigned int max, float dir) {
    if (position + dir < 0
        || position + dir >= max
        || rand() / double(RAND_MAX) < 0.05) {
        dir = round(rand() % 3) - 1;

        while (position + dir < 0 || position + dir >= max)
            dir = round(rand() % 3) - 1;
    }
    return dir;
}

int main() {
    int epoch = 0;
    SDL_Surface *screen;
    SDL_Event event;
    bool running = true;
    std::vector< std::vector< std::vector < float > > > previousScreen(height, std::vector< std::vector < float > >(width, std::vector<float>(3, 0)));
    std::vector< std::vector< std::vector < float > > > currentScreen(height, std::vector< std::vector < float > >(width, std::vector<float>(3, 0)));

    srand(time(NULL));

    screen = initGraphics(width, height);

    if (SDL_FillRect(screen, NULL, 0) == -1) {
        std::cout << "SDL_FillRect failed" << std::endl;
        return 0;
    }

    std::vector<float> xAgents(nbAgents, 0);
    std::vector<float> xAgentsDirs(nbAgents, 0);
    std::vector<float> yAgents(nbAgents, 0);
    std::vector<float> yAgentsDirs(nbAgents, 0);
    std::vector< std::vector < float> > colorAgents(nbAgents, std::vector<float>(3, 200));

    for (unsigned short i = 0; i < nbAgents; ++i) {
        xAgents[i] = rand() % width;
        yAgents[i] = rand() % height;
        for (unsigned short colorIndex = 0; colorIndex < 3; ++colorIndex)
            colorAgents[i][colorIndex] = rand() % 256;
    }

    while (running) {
        std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
        std::cout << "epoch: " << epoch << std::endl;

        for (unsigned short i = 0; i < nbAgents; ++i) {
            for (unsigned short colorIndex = 0; colorIndex < 3; ++colorIndex)
                previousScreen[yAgents[i]][xAgents[i]][colorIndex] = colorAgents[i][colorIndex];
            xAgentsDirs[i] = newDirection(xAgents[i], width, xAgentsDirs[i]);
            yAgentsDirs[i] = newDirection(yAgents[i], height, yAgentsDirs[i]);
            xAgents[i] += xAgentsDirs[i];
            yAgents[i] += yAgentsDirs[i];
        }

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

        omp_set_dynamic(0);
        omp_set_num_threads(3);
        #pragma omp parallel for shared(screen, currentScreen, previousScreen)
        for (unsigned short i = 1; i < height - 1; ++i) {
            for (unsigned short j = 1; j < width - 1; ++j) {
                for (unsigned short k = 0; k < 3; ++k) {
                    #pragma omp atomic
                    currentScreen[i][j][k] = (
                        previousScreen[i - 1][j][k]
                        + previousScreen[i + 1][j][k]
                        + previousScreen[i][j - 1][k]
                        + previousScreen[i][j + 1][k]
                    ) / 2 - currentScreen[i][j][k];
                    currentScreen[i][j][k] = currentScreen[i][j][k] * damping;
                    if (currentScreen[i][j][k] < 0)
                        currentScreen[i][j][k] = 0;
                    if (currentScreen[i][j][k] > 255)
                        currentScreen[i][j][k] = 255;
                }
                setPixel(
                    screen, j, i,
                    SDL_MapRGB(
                        screen->format,
                        currentScreen[i][j][0],
                        currentScreen[i][j][1],
                        currentScreen[i][j][2]
                    )
                );
            }
        }

        if (SDL_Flip(screen) == - 1) {
            std::cout << "SDL_Flip failed" << std::endl;
            return 0;
        }

        previousScreen.swap(currentScreen);

        ++epoch;
        std::chrono::time_point<std::chrono::high_resolution_clock> finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "algo timing: " << elapsed.count() << std::endl;
    }

    SDL_Quit();
}
