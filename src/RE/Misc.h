#pragma once

#include "RE/Offset.h"

namespace RE
{
	inline bool IsTalking(Character* a_character)
	{
		using func_t = decltype(&IsTalking);
		static REL::Relocation<func_t> func{ RE::Offset::Character::IsTalking };
		return func(a_character);
	}

	inline void Say(
		TESObjectREFR* a_speaker,
		BSSoundHandle a_model,
		TESTopic* a_topic,
		PlayerCharacter* a_player,
		DialogueItem* a_dialogue)
	{
		using func_t = decltype(&Say);
		static REL::Relocation<func_t> func{ RE::Offset::TESObjectREFR::Say };
		func(a_speaker, a_model, a_topic, a_player, a_dialogue);
	}
}