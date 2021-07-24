#include "engine/engine.h"

int main()
{
	app::Engine engine;

	engine.StartupEngine();

	engine.Run(
		[&]()
		{

		});

	engine.CleanupEngine();

	return 0;
}