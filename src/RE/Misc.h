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
}