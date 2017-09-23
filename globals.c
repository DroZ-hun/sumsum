#include <SDL/SDL_image.h>
#include <assert.h>
#include "globals.h"

KeysDown KeyDown;
TextColors TextColor;
ApplicationProperties Application;
MainMenuProperties MainMenu;
Images Image;
MatchProperties Match;
PROGRAM_STATE *STATE_MAIN_MENU,
			  *STATE_VIEW_HIGHSCORES,
			  *STATE_PLAYING,
			  *STATE_VICTORY,
			  *STATE_DEFEAT,
			  *STATE_QUITTING;


void PROGRAM_SWITCH_STATE(PROGRAM_STATE *State)
{
	Application.NextState = State;
}

// Egy pszeudo véletlen számot ad vissza 0..(number-1) intervallumban (inkluzív).
// Ez egy biztonságos függvény, nem pozitív számmal hívva nullával tér vissza.
// Érdemes egyszer meghívni a srand(time(NULL)) utasítást a main függvényben a használata előtt.
int randmax(int number)
{
	if (number <= 0)
	{
		// Hasznos felhasználni a seed-et akkor is, ha rossz az input.
		// Azaz ha hívnak randmax()-ot, akkor hívódik rand() is.
		rand();
		return 0;
	}
	int random = rand() % number;

	// Csak a biztonság kedvéért ellenőrizzük az intervallumot.
	return (random >= 0 && random < number) ? random : 0;
}

const char* DifficultyToText(int difficulty)
{
	switch(difficulty)
	{
		case DIFFICULTY_EXTREME_WAR:
			return "< extreme war >";
			break;
		case DIFFICULTY_EXTREME:
			return "< extreme >";
			break;
		case DIFFICULTY_HARD:
			return "< hard >";
			break;
		case DIFFICULTY_NORMAL:
			return "< normal >";
			break;
		default:
			return "< easy >";
	}
}

Color GetExtremeDifficultyColor(ExtremeAi extremeAi)
{
	switch(extremeAi)
	{
		case AI_ULTIMATE:
			if (Match.PlayerColor == BLUE)
			{
				return RED;
			}
			return BLUE;
			break;
		case AI_WARRIOR:
			if (Match.PlayerColor == WHITE)
			{
				return PINK;
			}
			return WHITE;
			break;
		case AI_AGGRESSIVE_WARRIOR:
			if (Match.PlayerColor == BLACK)
			{
				return PURPLE;
			}
			return BLACK;
			break;
		default:
			assert(0);
	}
}


SDL_Surface *IMG_Load_Optimized(char *filename)
{
	SDL_Surface *raw = IMG_Load(filename);
	SDL_Surface *optimized = SDL_DisplayFormat(raw);
	SDL_FreeSurface(raw);

	return optimized;
}
