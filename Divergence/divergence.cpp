/*
 * Divergence (a Sokoban (or Sokouban if you're a purist) clone)
 * Copyright (C) 2020 Guywan
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
#include <string>
#include <fstream>

#include <string.h>
#include <stdio.h>

const int LEFT = SDL_SCANCODE_LEFT;
const int UP = SDL_SCANCODE_UP;
const int RIGHT = SDL_SCANCODE_RIGHT;
const int DOWN = SDL_SCANCODE_DOWN;
const int KEY_R = SDL_SCANCODE_R;

const int FLOOR = 1;
const int WALL = 2;

struct Point
{
    int x, y;
};

struct Cell
{
    int type;
    bool isGoal, hasBox, onGoal;
};

struct Level
{
    int width, height, goals;
    Point player;
    std::vector<std::vector<Cell>> map;
};

// Function prototypes.
bool initLevels();
bool init();
Level loadLevel(const std::string&);
bool update(int, Level&);
Point move(int, const Point&);
bool moveBox(int, const Point&, Level&);
void render(const Level&);

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool fullscreen = true;

int W_WIDTH = 0;
int W_HEIGHT = 0;

int cellSize = 0, xp = 0, yp = 0;

std::vector<std::string> levels;

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

    if(!initLevels()) return 1;

    if(!init()) return 1;

    if(fullscreen)
    {
        // Set the W_WIDTH and W_HEIGHT variables.
        SDL_GL_GetDrawableSize(window, &W_WIDTH, &W_HEIGHT);
    }

    // Load first level.
    int curLevel = 0;
    Level level = loadLevel(levels[curLevel]);

    // Render initial state.
    render(level);

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
                            // Move the player, if possible.
                            // If level is complete, load the next one.
                            if(update(event.key.keysym.scancode, level))
                            {
                                // Wait a bit, for esoteric reasons.
                                SDL_Delay(800);

                                if(++curLevel < levels.size())
                                {
                                    level = loadLevel(levels[curLevel]);
                                    render(level);
                                }
                                else
                                {
                                    printf("All levels completed.\n");
                                    running = false;
                                }
                            }
                            break;

                        case KEY_R:
                            // Restart the current level.
                            level = loadLevel(levels[curLevel]);
                            render(level);
                    }
            }
        }
    }

    // Destroy the renderer, window, and quit SDL.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

bool initLevels()
{
    // Load definition strings of Divergence
    // levels from the level file.
    std::ifstream levelFile;

    levelFile.open("levels");
    if(!levelFile.is_open())
    {
        printf("Error: could not open 'levels'!");
    }
    else
    {
        std::string line;
        std::string level = "";
        while(std::getline(levelFile, line))
        {
            if(line.compare(",") == 0)
            {
                level.pop_back();
                levels.push_back(level);
                level = "";
            }
            else
            {
                level += line + "|";
            }
        }

        levelFile.close();

        return true;
    }

    return false;
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

Level loadLevel(const std::string &def)
{
    // Create a Divergence level from a definition string.
    Level level;
    level.goals = 0;
    level.map.push_back(std::vector<Cell>());

    int width = 0, height = 0, x = 0;
    for(unsigned int i = 0; i < def.length(); i++)
    {
        switch(def.at(i))
        {
            case '.': // Goal.
                level.map[height].push_back(Cell{FLOOR, true, false, false});
                level.goals++;
                break;

            case '$': // Box.
                level.map[height].push_back(Cell{FLOOR, false, true, false});
                break;

            case '*': // Box over goal.
                level.map[height].push_back(Cell{FLOOR, true, true, true});
                break;

            case '#': // Wall.
                level.map[height].push_back(Cell{WALL, false, false, false});
                break;

            case '@': // Player.
                level.player = Point{x, height};
                level.map[height].push_back(Cell{FLOOR, false, false, false});
                break;

            case '&': // Player over goal.
                level.player = Point{x, height};
                level.map[height].push_back(Cell{FLOOR, true, false, false});
                level.goals++;
                break;

            case '|': // Start a new row.
                height++;
                if(x > width) width = x;

                x = -1;
                level.map.push_back(std::vector<Cell>());
                break;

            default: // Empty floor.
                level.map[height].push_back(Cell{FLOOR, false, false, false});
                break;
        }

        x++;
    }

    level.width = width;
    level.height = ++height;

    // Determine cellSize based on level and window dimensions.
    // Allows the drawn map to scale to the window size.
    cellSize = (int)std::min(W_WIDTH / width, W_HEIGHT / height);

    // Determine x and y padding, which are
    // used to centre the level within the window.
    xp = (int)((W_WIDTH - (cellSize * width)) / 2);
    yp = (int)((W_HEIGHT - (cellSize * height)) / 2);

    return level;
}

bool update(int direction, Level &level)
{
    Point dest = move(direction, level.player);
    if(level.map[dest.y][dest.x].type != WALL)
    {
        // If the player moves into a box, we try to push that box.
        if(level.map[dest.y][dest.x].hasBox && moveBox(direction, dest, level))
        {
            level.player = dest;

            render(level);

            // Check if the level has been completed.
            if(level.goals == 0)
            {
                return true;
            }
        }
        else if(!level.map[dest.y][dest.x].hasBox)
        {
            level.player = dest;

            render(level);
        }
    }

    return false;
}

Point move(int direction, const Point &src)
{
    switch(direction)
    {
        case LEFT: return Point{src.x - 1, src.y};
        case UP: return Point{src.x, src.y - 1};
        case RIGHT: return Point{src.x + 1, src.y};
        case DOWN: return Point{src.x, src.y + 1};
    }
}

bool moveBox(int direction, const Point &src, Level &level)
{
    // We move the box if the destination does not
    // contain a wall or another box.
    Point dest = move(direction, src);
    if(level.map[dest.y][dest.x].type != WALL && !level.map[dest.y][dest.x].hasBox)
    {
        level.map[src.y][src.x].hasBox = false;
        level.map[dest.y][dest.x].hasBox = true;

        // Increment remaining goals if the box was pushed off a goal.
        if(level.map[src.y][src.x].isGoal)
        {
            level.map[src.y][src.x].onGoal = false;
            level.goals++;
        }

        // Decrement remaining goals if the box was pushed onto a goal.
        if(level.map[dest.y][dest.x].isGoal)
        {
            level.map[dest.y][dest.x].onGoal = true;
            level.goals--;
        }

        return true;
    }

    return false;
}

void render(const Level &level)
{
    // Fill the entire surface with black.
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    // Used to draw goals, which need to be comparatively smaller than boxes.
    int quarter = cellSize / 4;

    for(unsigned int y = 0; y < level.height; y++)
    {
        for(unsigned int x = 0; x < level.map[y].size(); x++)
        {
            if(level.map[y][x].type == FLOOR)
            {
                if(level.map[y][x].hasBox)
                {
                    // Determine what colour the box should be.
                    // If the box is on a goal, draw it in green
                    // to differentiate it from other boxes.
                    if(level.map[y][x].onGoal)
                    {
                        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
                    }
                    else
                    {
                        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
                    }

                    // Draw the boxes.
                    SDL_Rect r{x * cellSize + xp, y * cellSize + yp, cellSize - 1, cellSize - 1};
                    SDL_RenderFillRect(renderer, &r);
                }
                else if(level.map[y][x].isGoal)
                {
                    // Draw the goals.
                    SDL_Rect r{x * cellSize + quarter + xp, y * cellSize + quarter + yp, cellSize - quarter * 2, cellSize - quarter * 2};
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
                    SDL_RenderFillRect(renderer, &r);
                }
            }
            else
            {
                // Draw the walls.
                SDL_Rect r{x * cellSize + xp, y * cellSize + yp, cellSize - 1, cellSize - 1};
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }

    // Draw the player.
    SDL_Rect r{level.player.x * cellSize + xp, level.player.y * cellSize + yp, cellSize - 1, cellSize - 1};
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &r);

    // Update the window with the rendering performed.
    SDL_RenderPresent(renderer);
}
