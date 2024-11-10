#include "settings/JSONSettings.h"

#include "hooks/hooks.h"
#include "utilities/utilities.h"

namespace {
	static std::vector<std::string> findJsonFiles()
	{
		static constexpr std::string_view directory = R"(Data/SKSE/Plugins/ConsiderateFollowers)";
		std::vector<std::string> jsonFilePaths;
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			if (entry.is_regular_file() && entry.path().extension() == ".json") {
				jsonFilePaths.push_back(entry.path().string());
			}
		}

		std::sort(jsonFilePaths.begin(), jsonFilePaths.end());
		return jsonFilePaths;
	}
}

namespace Settings::JSON
{
	void Read() {
		std::vector<std::string> paths{};
		const auto singleton = Hooks::DialogueItemConstructorCall::GetSingleton();
		try {
			paths = findJsonFiles();
		}
		catch (const std::exception& e) {
			logger::warn("Caught {} while reading files.", e.what());
			return;
		}
		if (paths.empty()) {
			logger::info("No settings found");
			return;
		}

		for (const auto& path : paths) {
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				JSONReader.parse(rawJSON, JSONFile);
			}
			catch (const Json::Exception& e) {
				logger::warn("Caught {} while reading files.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				logger::error("Caught unhandled exception {} while reading files.", e.what());
				continue;
			}

			if (!JSONFile.isObject()) {
				logger::warn("Warning: <{}> is not an object. File will be ignored.", path);
				continue;
			}

			const auto& whitelists = JSONFile["whitelist"];
			if (whitelists) {
				if (whitelists.isArray()) {
					for (const auto& entry : whitelists) {
						if (!entry.isString()) {
							logger::warn("File <{}> has bad entry data in whitelists (not a string).", path);
							continue;
						}

						const auto candidateQuest = Utilities::Forms::GetFormFromString<RE::TESQuest>(entry.asString());
						const auto candidateActor = Utilities::Forms::GetFormFromString<RE::TESNPC>(entry.asString());
						if (candidateActor) {
							logger::debug("Whitelisted {} - {}", entry.asString(), candidateActor->GetName());
							singleton->RegisterWhitelistedActor(candidateActor);
						}
						else if (candidateQuest) {
							logger::debug("Whitelisted {} - {}", entry.asString(), candidateQuest->GetName());
							singleton->RegisterWhitelistedQuest(candidateQuest);
						}
						else {
							logger::warn("Warning: {} in <{}> did not match a form in a loaded plugin.", entry.asString(), path);
						}
					}
				}
				else {
					logger::error("Files <{}> has whitelists defined, but it is not an array.", path);
				}
			}
		}
	}
}