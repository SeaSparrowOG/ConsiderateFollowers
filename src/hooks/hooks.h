#pragma once

#include "utilities/utilities.h"

namespace Hooks {
	void Install();

	class DialogueItemConstructorCall :
		public Utilities::Singleton::ISingleton<DialogueItemConstructorCall> {
	public:
		void Install();
		void SetMaxDistance(double a_newDistance);
		void RegisterWhitelistedActor(const RE::TESNPC* a_actor);
		void RegisterWhitelistedQuest(const RE::TESQuest* a_quest);

	private:
		static RE::DialogueItem* CreateDialogueItem(
			RE::DialogueItem* a_dialogueItem,
			RE::TESQuest* a_quest,
			RE::TESTopic* a_topic,
			RE::TESTopicInfo* a_topicInfo,
			RE::TESObjectREFR* a_speaker);

		bool ShouldFollowerRespond(RE::Actor* a_speaker, RE::TESTopic* a_topic);

		// This isn't perfect, but is better than closestConversation. The limitation is simple:
		// Actor A starts talking, is closest.
		// Actor B starts talking, is NOT closest.
		// Player moves closer to Actor B before Actor B speaks again, and internally we will
		// test against Actor A instead of B.
		void UpdateInternalClosestConversation(RE::Actor* a_speaker);

		inline static REL::Relocation<decltype(&CreateDialogueItem)> _createDialogueItem;

		RE::Actor* closestSpeaker{ nullptr };
		float maximumDistance{ 500.0f };

		std::vector<const RE::TESNPC*> whitelistedActors;
		std::vector<const RE::TESQuest*> whitelistedQuests;
	};
}