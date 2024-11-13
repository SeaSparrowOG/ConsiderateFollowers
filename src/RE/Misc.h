#pragma once

#include "RE/Offset.h"

#include "utilities/utilities.h"

namespace RE
{

	inline void ClearGreetingInfoData(AIProcess* a_this) {
		using func_t = decltype(&ClearGreetingInfoData);
		static REL::Relocation<func_t> func{ Offset::AIProcess::ClearGreetingInfoData };
		func(a_this);
	}

	inline bool PlayerIsSleepingOrResting(PlayerCharacter* a_player) {
		using func_t = decltype(&PlayerIsSleepingOrResting);
		static REL::Relocation<func_t> func{ RE::Offset::PlayerCharacter::GetSleepRestState };
		return func(a_player);
	}

	inline bool GetCharacterEssential(Character* a_character) {
		using func_t = decltype(&GetCharacterEssential);
		static REL::Relocation<func_t> func{ RE::Offset::Character::GetEssential };
		return func(a_character);
	}

	inline RE::PROCESS_TYPE GetCharacterProcessLevel(Character* a_character) {
		using func_t = decltype(&GetCharacterProcessLevel);
		static REL::Relocation<func_t> func{ RE::Offset::Character::GetProccess };
		return func(a_character);
	}

	inline bool IsTalking(Character* a_character)
	{
		using func_t = decltype(&IsTalking);
		static REL::Relocation<func_t> func{ RE::Offset::Character::IsTalking };
		return func(a_character);
	}

	inline DialogueItem* CreateDialogueItem(
		TESTopic* a_topic,
		TESObjectREFR* a_speaker,
		Character* a_speakAs = nullptr,
		TESTopicInfo* a_topicInfo = nullptr,
		TESTopic* a_topic2 = nullptr,
		int64_t zero = 0) {

		using func_t = decltype(&CreateDialogueItem);
		static REL::Relocation<func_t> func{ RE::Offset::TESTopic::CreateDialogueItem };
		return func(a_topic, a_speaker, a_speakAs, a_topicInfo, a_topic2, zero);
	}

	inline void DialogueItemFirstResponse(DialogueItem* a_this) {
		using func_t = decltype(&DialogueItemFirstResponse);
		static REL::Relocation<func_t> func{ RE::Offset::DialogueItem::FirstResponse };
		func(a_this);
	}

	inline void RunResult(DialogueItem* a_this, int32_t a_resultScript, bool a_something) {
		using func_t = decltype(&RunResult);
		static REL::Relocation<func_t> func{ RE::Offset::DialogueItem::RunResult };
		func(a_this, a_resultScript, a_something);
	}

	inline void AIProcessProccessGreet(
		AIProcess* a_this,
		Character* a_speaker,
		TESTopic* a_topic = nullptr,
		TESTopicInfo* a_topicInfo = nullptr,
		bool a5 = false,
		bool a6 = false,
		bool a7 = true,
		bool a8 = true) {

		using func_t = decltype(&AIProcessProccessGreet);
		static REL::Relocation<func_t> func{ RE::Offset::AIProcess::ProcessGreet };
		func(a_this, a_speaker, a_topic, a_topicInfo, a5, a6, a7, a8);
	}
}