#include "core/application.h"

int main()
{
	flower::application app{ };

	app.initialize();
	app.loop();
	app.destroy();

	return 0;
}