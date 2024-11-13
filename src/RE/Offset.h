#pragma once

namespace RE
{
	namespace Offset
	{
		namespace AIProcess
		{
			constexpr auto ClearGreetingInfoData = REL::ID(39317);
			constexpr auto ProcessGreet = REL::ID(39162);
		}

		namespace Character
		{
			constexpr auto IsTalking = REL::ID(37266);
			constexpr auto GetProccess = REL::ID(37965);
			constexpr auto GetEssential = REL::ID(37228);
		}

		namespace DialogueItem
		{
			constexpr auto FirstResponse = REL::ID(35222);
			constexpr auto RunResult = REL::ID(35225);
		}

		namespace PlayerCharacter
		{
			constexpr auto GetSleepRestState = REL::ID(40413);
		}

		namespace TESTopic
		{
			constexpr auto CreateDialogueItem = REL::ID(25541);
		}
	}
}