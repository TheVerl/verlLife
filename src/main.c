// Standard library
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <SDL2/SDL.h>

// Own code
#include "error.h"
#include "game.h"
#include "graphic.h"

// Constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int LEVEL_WIDTH = 400;
const int LEVEL_HEIGHT = 400; 

// Quit programme
void end()
{
	// Deallocate surface, destroy window, etc, & quit
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();
}

// Save address of variable that has its memory dynamically allocated, so that it can be freed at quit
void saveAddress(void*** addressList, int* addressCount, void* selectThing)
{
	*addressCount = *addressCount + 1;
	*addressList = (void**)realloc(*addressList, *addressCount * sizeof(void*));
	if (addressList == NULL)
	{
		errorHandle(E_REALLOC, "addressList");
	}
	(*addressList)[*addressCount - 1] = selectThing;
	return;
}

int main (int argc, char* argv[])
{
	// Initialise SDL
	if (graphicInit())
	{
		errorHandle(E_INIT);
	}
	else {
		// Array of pointers to memory allocated variables, to be freed on quit
		void** addressList = (void**)malloc(1 * sizeof(void*));
		if (addressList == NULL)
		{
			errorHandle(E_MEM, "addressList");
		}
		int addressListCount = 0;

		// Declare entity list counter
		int mainEntityListCount = 0;

		// Allocate memory for entity list, then initialise its pointer elements
		entity** mainEntityList = (entity**)malloc(10010 * sizeof(entity*));
		if (mainEntityList == NULL)
		{
			errorHandle(E_MEM, "mainEntityList");
		}
		for (int i = 0; i < 10010; i++)
		{
			mainEntityList[i] = malloc(sizeof(entity));
			mainEntityList[i]->thing = NULL;
			mainEntityList[i]->form = 0;
			mainEntityList[i]->x = 0;
			mainEntityList[i]->y = 0;
			mainEntityList[i]->initX = 0;
			mainEntityList[i]->initY = 0;
		}
		saveAddress(&addressList, &addressListCount, (void*)mainEntityList);
		// Allocate memeory for cell grid, then initialise its pointer elements
		entity** cellGrid = (entity**)malloc(10000 * sizeof(entity*));
		if (cellGrid == NULL)
		{
			errorHandle(E_MEM, "cellGrid");
		}
		cellGrid = initialiseCellGrid(cellGrid, LEVEL_WIDTH, LEVEL_HEIGHT, mainEntityList, &mainEntityListCount, &addressList, &addressListCount);
		saveAddress(&addressList, &addressListCount, (void*)&cellGrid);

		// Set up camera
		SDL_Rect cameraRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		entity* camera = createEntity(&cameraRect, T_CAMERA, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, mainEntityList, &mainEntityListCount);

		// Create an event to be handled
		SDL_Event e;

		// Main loop
		while (!quit)
		{
			// Event handler loop
			while(SDL_PollEvent(&e) != 0)
			{
				// Free memory on quit
				if (e.type == SDL_QUIT)
				{
					for (int i = 0; i <= addressListCount; i++)
					{
						free(addressList[i]);
					}
					quit = true;
				//Handle key inputs
				} else if (e.type == SDL_KEYDOWN) {
					updateKeyStates(true, e.key.keysym.sym, cellGrid);
				} else if (e.type == SDL_KEYUP) {
					updateKeyStates(false, e.key.keysym.sym, cellGrid);
				} else if (keyStates.start == false) {
					if (e.type == SDL_MOUSEBUTTONDOWN) {
						updateCellStates(true, e.button.x, e.button.y, cellGrid);
					} else if (e.type == SDL_MOUSEBUTTONUP) {
					updateCellStates(false, e.button.x, e.button.y, cellGrid);
					}
				}
			}
			updateGame(camera, mainEntityList);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

			// Simulate the game
			if (keyStates.start)
			{
				calculate(cellGrid);
				simulate(cellGrid);
			}

			// Loop through cells and display them
			for (int i = 0; i < ((LEVEL_WIDTH / 10) * (LEVEL_HEIGHT / 10)); i++)
			{
				entity* cellPtr = cellGrid[i];
				cellThing* thingPtr = (cellThing*)cellPtr->thing;
				SDL_Rect* rectPtr = thingPtr->graphics;
				cellData* dataPtr = thingPtr->data;
				if (dataPtr->lifeState) {
					SDL_RenderFillRect(renderer, rectPtr);
				} else {
					SDL_RenderDrawRect(renderer, rectPtr); 
				}
			}
			SDL_RenderPresent(renderer);
			SDL_Delay(17);
		}
	}
	// Close programme
	end();
	return 0;
}
