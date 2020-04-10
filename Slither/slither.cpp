/*
 * Slither (a Snake clone)
 * Copyright (C) 2020 Czespo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <SDL.h>

#include <cstdlib>
#include <vector>
#include <iostream>
#include <ctime>

#include <string.h>
#include <stdio.h>

struct Point
{
    int x, y;
};

// Function prototypes.
bool init();
void update();
void render();

const int B_WIDTH = 20;
const int B_HEIGHT = 20;

const int DELAY = (int) 1000 / 10;

const int LEFT = SDL_SCANCODE_LEFT;
const int UP = SDL_SCANCODE_UP;
const int RIGHT = SDL_SCANCODE_RIGHT;
const int DOWN = SDL_SCANCODE_DOWN;
const int KEY_P = SDL_SCANCODE_P;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool fullscreen = true;

int W_WIDTH = 0;
int W_HEIGHT = 0;

int cellSize = 0, xp = 0, yp = 0;

int direction = RIGHT;

int length = 3;
std::vector<Point> body;

Point food{0, 0};

int main(int argc, char* args[])
{
    // Allow a `-w` flag to launch in windowed mode.
    // Can be followed by width and height: `-w 800 600`.
    // Defaults to 800x600.
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(args[i], "-w") == 0)
        {
            fullscreen = false;
            if(i + 2 < argc)
            {
                W_WIDTH = atoi(args[i + 1]);
                W_HEIGHT = atoi(args[i + 2]);
            }
            else
            {
                W_WIDTH = 800;
                W_HEIGHT = 600;
            }
        }
    }

    if(!init()) return 1;

    if(fullscreen)
    {
        // Set the W_WIDTH and W_HEIGHT variables.
        SDL_GL_GetDrawableSize(window, &W_WIDTH, &W_HEIGHT);
    }

    // Determine cell size based on board and window dimensions.
    // Allows the drawn board to scale to the window size.
    cellSize = (int)std::min(W_WIDTH / B_WIDTH, W_HEIGHT / B_HEIGHT);

    // Determine x and y padding, which are
    // used to centre the board within the window.
    xp = (int)((W_WIDTH - (cellSize * B_WIDTH)) / 2);
    yp = (int)((W_HEIGHT - (cellSize * B_HEIGHT)) / 2);

    // Fill the window surface with gray.
    SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0x88, 0xFF);
    SDL_RenderClear(renderer);

    // Seed the (pseudo)random number generator.
    srand(time(NULL));

    // Initialize the snake body.
    for(int k = 0; k < length; k++)
    {
        body.push_back(Point{(int)B_WIDTH / 2 - k, (int)B_HEIGHT / 2});
    }

    // Initialize the food location.
    food.x = (int)(rand() % B_WIDTH);
    food.y = (int)(rand() % B_HEIGHT);

    bool paused = false;

    // Main game loop.
    bool running = true;
    while(running)
    {
        // Handle events.
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:

                    switch((int) event.key.keysym.scancode)
                    {
                        case SDL_SCANCODE_ESCAPE:
                            running = false;
                            break;

                        case LEFT:
                        case UP:
                        case RIGHT:
                        case DOWN:
                            direction = event.key.keysym.scancode;
                            break;

                        case KEY_P:
                            paused = !paused;
                    }
            }
        }

        if(paused) continue;

        // Change game state.
        update();

        // Render the new state.
        render();

        // Wait for DELAY milliseconds before continuing.
        // Controls how fast the snake moves.
        SDL_Delay(DELAY);
    }

    // Destroy the renderer, window, and quit SDL.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

bool init()
{
    // Initialize SDL.
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialise! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        // Create window.
        int flags = SDL_WINDOW_SHOWN;
        if(fullscreen)
        {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        window = SDL_CreateWindow("Slither", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W_WIDTH, W_HEIGHT, flags);
        if(window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            
            SDL_Quit();
        }
        else
        {
            // Set the renderer, which will be used as
            // the base for all drawing operations.
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if(renderer == NULL)
            {
                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());

                SDL_DestroyWindow(window);
                SDL_Quit();
            }
            else
            {
                return true;
            }
        }
    }

    return false;
}

void update()
{
    // Move the snake.
    int nx = body[0].x, ny = body[0].y;
    switch(direction)
    {
        case LEFT:
            nx = body[0].x - 1;
            break;

        case UP:
            ny = body[0].y - 1;
            break;

        case RIGHT:
            nx = body[0].x + 1;
            break;

        case DOWN:
            ny = body[0].y + 1;
            break;
    }

    // If the snake tries to go off the edge
    // of the board, wrap it around.
    if(nx >= B_WIDTH)
    {
        nx -= B_WIDTH;
    }
    else if(nx < 0)
    {
        nx += B_WIDTH;
    }
    else if(ny >= B_HEIGHT)
    {
        ny -= B_HEIGHT;
    }
    else if(ny < 0)
    {
        ny += B_HEIGHT;
    }

    // Move the snake by adding a new head.
    body.insert(body.begin(), Point{nx, ny});

    // Check if the snake is eating food.
    if(body[0].x == food.x && body[0].y == food.y)
    {
        // Increment length.
        length++;

        // Set food to a random location within the board's dimensions.
        food.x = (int)(rand() % B_WIDTH);
        food.y = (int)(rand() % B_HEIGHT);
    }
    else
    {
        // If the snake hasn't eaten, remove the tail.
        body.pop_back();
    }

    // Check if the snake is eating itself.
    // Start with the third part, since the
    // snake cannot eat any part before that.
    for(int i = 2; i < body.size(); i++)
    {
        if(body[0].x == body[i].x && body[0].y == body[i].y)
        {
            // Set length to 3 and trim body.
            length = 3;
            while(body.size() > length)
            {
                body.pop_back();
            }

            break;
        }
    }
}

void render()
{
    // Do rendering.

    // Fill the window surface with gray.
    SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0x88, 0xFF);
    SDL_RenderClear(renderer);

    // Fill the board with black.
    SDL_Rect r{xp, yp, cellSize * B_WIDTH, cellSize * B_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &r);

    // Draw the snake's head, in dark green.
    r = {body[0].x * cellSize + xp, body[0].y * cellSize + yp, cellSize - 1, cellSize - 1};
    SDL_SetRenderDrawColor(renderer, 0x00, 0x88, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &r);

    // Draw the rest of the body, in light green.
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    for(int i = 1; i < body.size(); i++)
    {
        r = {body[i].x * cellSize + xp, body[i].y * cellSize + yp, cellSize - 1, cellSize - 1};
        SDL_RenderFillRect(renderer, &r);
    }

    // Draw the food.
    r = {food.x * cellSize + xp, food.y * cellSize + yp, cellSize - 1, cellSize - 1};
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &r);

    // Update the window with the rendering performed.
    SDL_RenderPresent(renderer);
}
