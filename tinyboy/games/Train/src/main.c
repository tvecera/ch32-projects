
// ****************************************************************************
//
//                                  Train game
//
// ****************************************************************************

#include "../include.h"

int main(void)
{
	char ch;
	int i;

	// display splash screen
	Level = 0;
	InitLevel();
	KeyFlush();
	while ((ch = KeyGet()) == NOKEY) {}

	// randomize
	RandSeed += Time();

	// clear display
	DrawClear();

	// set first scene
	Level = LEVFIRST;
	Score = 0;

	while (True)
	{
		// demo
/*
		i = Level; // save current level

		while (True)
		{
			// play demo scene 0
			Level = 0;
			if (HelpLevel()) break; // break

#ifdef AUTOMODE // automode - run levels to check solutions (help completes scene)
			break;
#endif

			// play random scene
			Level = RandU8MinMax(1, LEVNUM);
			if (HelpLevel()) break; // break
		}
*/

		// return current level
//		Level = i;
		InitLevel();

		// main loop
		while (True)
		{
			// game loop - called every single game step
			if (GameLoop()) break;

			// wait step
			WaitStep();
		}
	}
}
