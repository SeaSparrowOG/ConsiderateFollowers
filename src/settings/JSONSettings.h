#pragma once

namespace Settings
{
	namespace JSON
	{
		class Holder :
			public ISingleton<Holder>
		{
		public:
			bool Read();

		private:
			std::uint8_t parserVersion{ 1 };
			std::string MIN_VERSION_FIELD{ "MinimumVersion" };
			std::string WHITELIST_FIELD{ "Whitelist" };

			RE::TESDataHandler* dataHandler{ nullptr };
			std::vector<const RE::TESNPC*> configNPCs{};

			bool ReadConfig(const Json::Value& a_json);
			bool ReadWhitelist(const Json::Value& a_json);
			bool ResolveNPC(const Json::Value& a_json);
		};
	}
}