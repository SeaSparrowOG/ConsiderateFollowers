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

	inline bool Say(TESObjectREFR* a_this, DialogueItem* a_line)
	{
		using func_t = decltype(&Say);
		static REL::Relocation<func_t> func{ RE::Offset::TESObjectREFR::Say };
		return func(a_this, a_line);
	}
}