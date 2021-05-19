#include "core/application.h"
#include "pbr_deferred.h"
#include "asset_system/asset_pmx.h"
#include "asset_system/asset_vmd.h"

int main()
{
	flower::application app{ };

	app.initialize();

	flower::asset::VMDFile file_test{};
	flower::asset::ReadVMDFile(&file_test,"data/model/HCMiku v3 ver1.00/dance.vmd");
	
	std::shared_ptr<flower::iruntime_module> vk_module = std::make_shared<flower::graphics::pbr_deferred>(app.get_window());

	app.modules.push_back(vk_module);
	app.initialize_modules();
	
	app.loop();
	app.destroy();

	return 0;
}