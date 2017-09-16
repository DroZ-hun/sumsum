#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "globals.h"
#include "game_methods.h"

void AI(Town *this)
{
	if (randmax(Application.LPS_CAP) > 10)
		return;

	if (randmax(100) < Match.Difficulty && this->resources > WARRIOR_COST && this->peasants * WARRIOR_CONSUPTION_PER_MIN / PEASANT_PRODUCTION_PER_MIN > this->warriors)
		TownCreateWarrior(this);
	else
		if (randmax(100) < Match.Difficulty) TownCreatePeasant(this);

	Town *target = Match.TOWNS;
	int num = randmax(Match.NumberOfEnemies + 1) + 1;
	while (num--)
		target = target->NEXT;

	if (randmax(100) < Match.Difficulty && target->Color != this->Color && target != NULL)
		TownSendUnit(this, target);
}

void AIExtreme(Town *this)
{
	// TODO: újra ki kell próbálni az AI-t ha a katona paraszt erő arány kedvezőbb lesz a katonáknak.
	Unit *u_it;
	Town *t_it;
	// TODO: kipróbálni ugyanezen AI-k ellen.
	// TODO: kipróbálni mi történik ha PEASANT_WARRIOR_RATIO 5-re változik.
	int attacking_units_near = 0;
	int attacking_units = 0;

	int number_of_enemies_of_this_town = 0;
	int AI_MAGIC_CLOSENESS = 10;
	int AI_MAGIC_WEAK_POPULATION = 50;

	// extra 10 azért kell, hogy ne vibráljon a kép 999 és 1000 között.
	int AI_MAGIC_PEASANT_COUNT_FOR_RESOURCE_HEAVEN = 1010;

	for (u_it = Match.UNITS_BEGIN->NEXT; u_it != Match.UNITS_END; u_it = u_it->NEXT)
	{
		if (u_it->Color != this->Color && u_it->TO == this)
		{
			++attacking_units;
			if (u_it->ARRIVE - Application.LastRun < AI_MAGIC_CLOSENESS * SEC_TO_MS)
			{
				++attacking_units_near;
			}
		}
	}

	for (t_it = Match.TOWNS->NEXT; t_it != NULL; t_it = t_it->NEXT)
	{
		if (t_it->Color != this->Color)
		{
			++number_of_enemies_of_this_town;
		}
	}

	int safe_to_send_out_warriors = this->warriors >= (attacking_units_near+1) * UNIT_SIZE;

	// Feltételezi, hogy készítesz új katonát.
	int starving_warriors = ((this->warriors+1) * WARRIOR_CONSUPTION_PER_MIN - (this->peasants-1) * PEASANT_PRODUCTION_PER_MIN) / WARRIOR_CONSUPTION_PER_MIN;

	if (this->warriors >= UNIT_SIZE && (safe_to_send_out_warriors || starving_warriors > 0))
	{
		int enemy_population = 0;
		for (t_it = Match.TOWNS->NEXT; t_it != NULL; t_it = t_it->NEXT)
		{
			if (t_it->Color != this->Color)
			{
				enemy_population += t_it->warriors + t_it->peasants;
			}
		}
		// Célpont véletlen, de erősebb városoknak nagyobb az esélye, mivel fontosabb őket gyengíteni,
		// mint a gyengéket rohamozni.
		int target_person = randmax(enemy_population);
		Town *target = NULL;
		int enemy_search = 0;
		for (t_it = Match.TOWNS->NEXT; t_it != NULL; t_it = t_it->NEXT)
		{
			if (t_it->Color != this->Color)
			{
				// Csak hogy biztos legyen célpont, akkor is,
				// ha sehol sincs már katona és paraszt (üres ellenséges faluk esete).
				target = t_it;

				enemy_search += t_it->warriors + t_it->peasants;
				if (enemy_search >= target_person)
				{
					break;
				}
			}
		}

		// De az sem okos, ha engedjük a gyengéket felerősödni.
		int attack_weak = randmax(3) == 0;
		if(attack_weak)
		{
			Town* weaks[number_of_enemies_of_this_town];
			Town** weaks_end = weaks;
			for (t_it = Match.TOWNS->NEXT; attack_weak && t_it != NULL; t_it = t_it->NEXT)
			{
				if (t_it->Color == this->Color)
				{
					continue;
				}

				int population = t_it->peasants + t_it->warriors;
				if (population < AI_MAGIC_WEAK_POPULATION)
				{
					*weaks_end = t_it;
					++weaks_end;
				}
			}
			if(weaks_end != weaks)
			{
				target = weaks[randmax(weaks_end-weaks)];
			}
		}
		if (NULL != target)
		{
			TownSendUnit(this, target);
			return;
		}
	}

	if (this->peasants < AI_MAGIC_PEASANT_COUNT_FOR_RESOURCE_HEAVEN || starving_warriors >= 0)
	{
		// Csak hogy ne vibráljon 10->0... így marad mindig 2 számjegy.
		// Gyengébb lesz tőle ugyan egy picit, de még így is brutál.
		if (this->resources > 21)
		{
			TownCreatePeasant(this);
		}
	}
	else
	{
		TownCreateWarrior(this);
	}
}

void TTF_RenderText_Outline(SDL_Surface *dst, char *string, TTF_Font *font, int X, int Y, SDL_Color Ctxt, SDL_Color Cout)
{
	SDL_Surface *text;
	SDL_Rect r;

	text = TTF_RenderText_Solid(font, string, Cout);
	r.x = X - 1 - text->w / 2;
	r.y = Y - 1 - text->h / 2;
	SDL_BlitSurface(text, NULL, dst, &r);

	r.x = X + 1 - text->w / 2;
	r.y = Y + 1 - text->h / 2;
	SDL_BlitSurface(text, NULL, dst, &r);

	r.x = X - 1 - text->w / 2;
	r.y = Y + 1 - text->h / 2;
	SDL_BlitSurface(text, NULL, dst, &r);

	r.x = X + 1 - text->w / 2;
	r.y = Y - 1 - text->h / 2;
	SDL_BlitSurface(text, NULL, dst, &r);
	SDL_FreeSurface(text);

	text = TTF_RenderText_Solid(font, string, Ctxt);
	r.x = X - text->w / 2;
	r.y = Y - text->h / 2;
	SDL_BlitSurface(text, NULL, dst, &r);
	SDL_FreeSurface(text);
}

void TownCreatePeasant(Town *this)
{
	/*
	**	PARASZT KÉSZÍTÉS AZ ADOTT FALUBAN
	*/

	if (NULL == this)
	{
		return;
	}

	if (this->resources >= PEASANT_COST)
	{
		this->peasants += 1;
		this->resources -= PEASANT_COST;
	}
}

void TownCreateWarrior(Town *this)
{
	/*
	**	KATONA KÉSZÍTÉS AZ ADOTT FALUBAN
	*/

	if (NULL == this)
	{
		return;
	}

	if (this->resources >= WARRIOR_COST && this->peasants * PEASANT_PRODUCTION_PER_MIN >= (this->warriors + 1) * WARRIOR_CONSUPTION_PER_MIN)
	{
		this->warriors += 1;
		this->peasants -= 1;
		this->resources -= WARRIOR_COST;
	}
}



void TownSendUnit(Town *from, Town *to)
{
	if (NULL == from || NULL == to)
	{
		return;
	}

	if (from->warriors < UNIT_SIZE)
		return;

	/*
	**	KATONÁK KIKÜLDÉSE EGY FALUBÓL EGY MÁSIKBA
	*/

	Unit *prev = Match.UNITS_END->PREV;

	Unit *this = (Unit*)malloc(sizeof(Unit));
	this->PREV = prev;
	this->NEXT = Match.UNITS_END;
	prev->NEXT = this;
	Match.UNITS_END->PREV = this;

	this->FROM = from;
	this->TO = to;
	this->Color = from->Color;
	this->LEAVE = Application.LastRun;
	this->ARRIVE = this->LEAVE + (sqrt((from->Position.X - to->Position.X) * (from->Position.X - to->Position.X) + (from->Position.Y - to->Position.Y) * (from->Position.Y - to->Position.Y))) * UNIT_MARCHING_TIME;

	from->warriors -= UNIT_SIZE;
}

int CheckPlayerVictory()
{
	/*
	**	"JÁTÉKOS NYERT-E" FGV., 1-ET AD VISSZA, HA NYERT, MINDEN MÁS ESETBEN 0-T
	*/


	if (NOCOLOR == Match.PlayerColor)
	{
		return 0;
	}

	Town *t;
	for (t = Match.TOWNS->NEXT; t != NULL; t = t->NEXT)
		if (t->Color != Match.PlayerColor)
			return 0;

	Unit *u;
	for (u = Match.UNITS_BEGIN->NEXT; u != Match.UNITS_END; u = u->NEXT)
		if (u->Color != Match.PlayerColor)
			return 0;

	return 1;
}

int CheckPlayerDefeat()
{
	/*
	**	"JÁTÉKOS VESZTETT-E" FGV., 1-ET AD VISSZA, HA IGEN, MINDEN MÁS ESETBEN 0-T
	*/

	if (NOCOLOR == Match.PlayerColor)
	{
		return 0;
	}

	Town *t;
	for (t = Match.TOWNS->NEXT; t != NULL; t = t->NEXT)
		if (t->Color == Match.PlayerColor)
			return 0;

	Unit *u;
	for (u = Match.UNITS_BEGIN->NEXT; u != Match.UNITS_END; u = u->NEXT)
		if (u->Color == Match.PlayerColor)
			return 0;

	return 1;
}
