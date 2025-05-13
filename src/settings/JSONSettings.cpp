#include "settings/JSONSettings.h"

#include "DialogueManager/DialogueManager.h"

namespace
{
	std::vector<std::string> SplitString(const std::string& a_str, 
		const std::string& a_delimiter)
	{
		std::vector<std::string> result;
		size_t start = 0;
		size_t end = a_str.find(a_delimiter);

		while (end != std::string::npos) {
			result.push_back(a_str.substr(start, end - start));
			start = end + a_delimiter.length();
			end = a_str.find(a_delimiter, start);
		}

		result.push_back(a_str.substr(start));
		return result;
	}

	static bool IsOnlyHex(std::string_view a_str, bool a_requirePrefix = true)
	{
		if (!a_requirePrefix) {
			return std::ranges::all_of(a_str, [](unsigned char ch) {
				return std::isxdigit(ch);
				});
		}
		else if (a_str.compare(0, 2, "0x") == 0 || a_str.compare(0, 2, "0X") == 0) {
			return a_str.size() > 2 && std::all_of(a_str.begin() + 2, a_str.end(), [](unsigned char ch) {
				return std::isxdigit(ch);
				});
		}
		return false;
	}

	std::string ToLower(std::string_view a_str)
	{
		std::string result(a_str);
		std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
		return result;
	}

	template <class T>
	T ToNum(const std::string& a_str, bool a_hex = false)
	{
		const int base = a_hex ? 16 : 10;

		if constexpr (std::is_same_v<T, double>) {
			return static_cast<T>(std::stod(a_str, nullptr));
		}
		else if constexpr (std::is_same_v<T, float>) {
			return static_cast<T>(std::stof(a_str, nullptr));
		}
		else if constexpr (std::is_same_v<T, std::int64_t>) {
			return static_cast<T>(std::stol(a_str, nullptr, base));
		}
		else if constexpr (std::is_same_v<T, std::uint64_t>) {
			return static_cast<T>(std::stoull(a_str, nullptr, base));
		}
		else if constexpr (std::is_signed_v<T>) {
			return static_cast<T>(std::stoi(a_str, nullptr, base));
		}
		else {
			return static_cast<T>(std::stoul(a_str, nullptr, base));
		}
	}

	enum class ErrorCode : uint8_t
	{
		kSuccess,
		kMissingPlugin,
		kMissingForm,
		kBadStringID,
		kBadCast
	};

	template <typename T>
	std::pair<T*, ErrorCode> GetFormFromString(const std::string& a_str,
		RE::TESDataHandler* a_dataHandler)
	{
		ErrorCode errorCode = ErrorCode::kSuccess;
		T* response = nullptr;
		const auto splitID = SplitString(a_str, "|");
		const auto splitSize = splitID.size();

		if (splitSize == 2) {
			const auto& pluginName = splitID[0];
			const auto& rawID = splitID[1];
			if (!a_dataHandler->LookupModByName(pluginName)) {
				errorCode = ErrorCode::kMissingPlugin;
			}
			else if (!IsOnlyHex(rawID)) {
				errorCode = ErrorCode::kBadStringID;
			}

			if (errorCode == ErrorCode::kSuccess) {
				const auto formID = ToNum<RE::FormID>(rawID, true);
				auto* foundForm = a_dataHandler->LookupForm(formID, pluginName);
				if (!foundForm) {
					errorCode = ErrorCode::kMissingForm;
				} 
				else {
					response = skyrim_cast<T*>(foundForm);
					if (response) {
						return std::make_pair(response, errorCode);
					}
					else {
						errorCode = ErrorCode::kBadCast;
					}
				}
			}
		}
		auto* foundForm = RE::TESForm::LookupByEditorID(a_str);
		if (!foundForm) {
			errorCode = ErrorCode::kMissingForm;
			return std::make_pair(response, errorCode);
		}
		response = skyrim_cast<T*>(foundForm);
		if (!response) {
			errorCode = ErrorCode::kBadCast;
		}
		return std::make_pair(response, errorCode);
	}
}

namespace Settings::JSON
{
	bool Holder::Read() {
		logger::info("==========================================================");
		logger::info("JSON parser version: {}", parserVersion);
		std::string jsonFolder = fmt::format(R"(.\Data\SKSE\Plugins\{})"sv, Plugin::NAME);
		logger::info("Reading and validating project JSON files in {}.", jsonFolder);

		dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			logger::critical("Failed to fetch data handler. This will likely cause a crash later, treating state as invalid."sv);
			return false;
		}

		std::vector<std::string> paths{};
		try {
			for (const auto& entry : std::filesystem::directory_iterator(jsonFolder)) {
				if (entry.is_regular_file() && entry.path().extension() == ".json") {
					paths.push_back(entry.path().string());
				}
			}

			std::sort(paths.begin(), paths.end());
			logger::info("  >Found {} configuration files."sv, std::to_string(paths.size()));
		}
		catch (const std::exception& e) {
			logger::error("Caught {} while reading files.", e.what());
			return false;
		}
		if (paths.empty()) {
			logger::info("No settings found");
			return true;
		}

		for (const auto& path : paths) {
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				auto filename = path.substr(jsonFolder.size() + 1, path.size() - 1);
#ifdef NDEBUG
				if (filename == "_testConfig.json") {
					continue;
				}
#endif
				JSONReader.parse(rawJSON, JSONFile);
				logger::info("  >Reading config {}..."sv, filename);
				if (JSONFile.empty()) {
					logger::warn("  >Failed to read config {}."sv, filename);
					continue;
				}

				try {
					if (!ReadConfig(JSONFile)) {
						logger::warn("  >Treating config as invalid, no settings will be applied."sv);
						continue;
					}
				}
				catch (Json::Exception& e) {
					logger::error("    >Caught {} while reading file. File will be ignored."sv, e.what());
				}
				catch (std::exception& e) {
					logger::error("    >Caught unexpected exception {} while reading file. This should be reported to the mod page."sv, e.what());
				}
			}
			catch (const Json::Exception& e) {
				logger::error("  >Caught {} while reading file.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				logger::error("  >Caught unhandled exception {} while reading file.", e.what());
				continue;
			}
		}
		logger::info("Finished reading all settings."sv);
		return true;
	}

	bool Holder::ReadConfig(const Json::Value& a_json) {
		configNPCs.clear();
		const auto& minVersionField = a_json[MIN_VERSION_FIELD];
		if (minVersionField) {
			if (!minVersionField.isInt()) {
				logger::warn("    >Config specified a minimum version, but the {} field is not a number."sv, MIN_VERSION_FIELD);
				return false;
			}
			const auto requiredVer = minVersionField.asInt();
			if (requiredVer > parserVersion) {
				logger::warn("    >Config specifies minimum version {}, but parser is version {}. Update {}."sv, requiredVer, parserVersion, Plugin::NAME);
				return false;
			}
		}

		const auto& whitelistField = a_json[WHITELIST_FIELD];
		if (whitelistField) {
			if (!ReadWhitelist(whitelistField)) {
				return false;
			}

			if (configNPCs.empty()) {
				return true;
			}
			DialogueManager::Manager::GetSingleton()->RegisterWhitelistedNPCs(configNPCs);
		}
		logger::warn("    >Config does not have a {} field. This is likely an error with the config."sv, WHITELIST_FIELD);
		return false;
	}

	bool Holder::ReadWhitelist(const Json::Value& a_json) {
		if (a_json.isString()) {
			if (!ResolveNPC(a_json)) {
				return false;
			}
		}
		else if (a_json.isArray()) {
			for (const auto& arrayNPC : a_json) {
				if (!arrayNPC.isString()) {
					logger::warn("    >Config contains non-string entry in {} field."sv, WHITELIST_FIELD);
					return false;
				}
				if (!ResolveNPC(arrayNPC)) {
					return false;
				}
			}
		}
		else {
			logger::warn("    >Config contains {} field, but it is neither a string nor an array."sv, WHITELIST_FIELD);
			return false;
		}

		if (configNPCs.empty()) {
			logger::info("  >Config resulted in no whitelisted NPCs. This may be normal."sv);
			return true;
		}
		return true;
	}

	bool Holder::ResolveNPC(const Json::Value& a_json)
	{
		const auto jsonStringValue = a_json.asString();
		const auto foundNPC = GetFormFromString<RE::TESNPC>(jsonStringValue, dataHandler);
		if (!foundNPC.first) {
			switch (foundNPC.second) {
			case ErrorCode::kMissingPlugin:
				logger::info("      >Entry {} specifies a plugin not in the load order. This is not a critical error."sv, jsonStringValue);
				break;
			case ErrorCode::kMissingForm:
				logger::warn("      >Entry {} specifies form in a plugin, but that plugin does not include that form."sv, jsonStringValue);
				break;
			case ErrorCode::kBadCast:
				logger::warn("      >Entry {} specifies form in a plugin, but the form in that plugin is NOT an actorbase."sv, jsonStringValue);
				break;
			case ErrorCode::kBadStringID:
				logger::warn("      >Entry {} has an invalid formID."sv, jsonStringValue);
				break;
			default:
				logger::warn("      >Failed to resolve entry {}."sv, jsonStringValue);
			}
		}
		else {
			configNPCs.push_back(foundNPC.first);
		}
		return true;
	}
}