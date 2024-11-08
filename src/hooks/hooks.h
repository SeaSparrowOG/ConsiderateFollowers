#pragma once

#include "utilities/utilities.h"

namespace Hooks {
	void Install();

	class DialogueItemConstructorCall :
		public Utilities::Singleton::ISingleton<DialogueItemConstructorCall> {
	public:
		void Install();

	private:
		static RE::DialogueItem* CreateDialogueItem(
			RE::DialogueItem* a_dialogueItem,
			RE::TESQuest* a_quest,
			RE::TESTopic* a_topic,
			RE::TESTopicInfo* a_topicInfo,
			RE::TESObjectREFR* a_speaker);

		inline static REL::Relocation<decltype(&CreateDialogueItem)> _createDialogueItem;
	};
}