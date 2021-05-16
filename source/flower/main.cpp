#include "core/application.h"
#include "pbr_deferred.h"
#include "asset_system/asset_pmx.h"

int main()
{
	flower::application app{ };

	app.initialize();
	
	std::shared_ptr<flower::iruntime_module> vk_module = std::make_shared<flower::graphics::pbr_deferred>(app.get_window());

	app.modules.push_back(vk_module);
	app.initialize_modules();
	
	app.loop();
	app.destroy();

	return 0;
}