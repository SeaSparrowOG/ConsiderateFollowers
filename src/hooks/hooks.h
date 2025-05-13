#pragma once

namespace Hooks {
	bool Install();

	class DialogueItemConstructorCall :
		public ISingleton<DialogueItemConstructorCall> {
	public:
		bool Install();

	private:

		static RE::DialogueItem* Thunk(
			RE::DialogueItem* a_dialogueItem,
			RE::TESQuest* a_quest,
			RE::TESTopic* a_topic,
			RE::TESTopicInfo* a_topicInfo,
			RE::TESObjectREFR* a_speaker);

		inline static REL::Relocation<decltype(&Thunk)> _func;
	};

	class PlayerUpdateListener :
		public ISingleton<PlayerUpdateListener>
	{
	public:
		bool Install();

	private:
		inline static void Thunk(RE::PlayerCharacter* a_this, float a_delta);
		inline static REL::Relocation<decltype(&Thunk)> _func;
		static constexpr std::size_t idx{ 0xAD }; //Update

		inline static float internalCounter{ 0.0f };
		inline static float timeBetweenAttempts{ 10.0f };
	};
}