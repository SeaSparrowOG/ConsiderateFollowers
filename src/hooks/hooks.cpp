#include "Hooks/hooks.h"

namespace Hooks {
	void DialogueItemConstructorCall::Install()
	{
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> playerGoldTarget{ REL::ID(25541), 0xE2 };
		_createDialogueItem = trampoline.write_call<5>(playerGoldTarget.address(), &CreateDialogueItem);
	}

	RE::DialogueItem* DialogueItemConstructorCall::CreateDialogueItem(
		RE::DialogueItem* a_this, 
		RE::TESQuest* a_quest,
		RE::TESTopic* a_topic,
		RE::TESTopicInfo* a_topicInfo,
		RE::TESObjectREFR* a_speaker)
	{
		return _createDialogueItem(a_this, a_quest, a_topic, a_topicInfo, a_speaker);
	}
}