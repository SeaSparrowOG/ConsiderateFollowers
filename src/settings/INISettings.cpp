#include "settings/INISettings.h"

#include "hooks/hooks.h"
#include "utilities/utilities.h"
#include <SimpleIni.h>

namespace Settings::INI
{
	void Read() {
		const auto singleton = Hooks::DialogueItemConstructorCall::GetSingleton();
		CSimpleIniA ini{};
		ini.SetUnicode();
		ini.LoadFile(fmt::format(R"(.\Data\SKSE\Plugins\{}.ini)", Plugin::NAME).c_str());

		const auto newDistance = ini.GetDoubleValue("General", "fMaxConversationDistance", 400.0);
		singleton->SetMaxDistance(newDistance);
	}
}