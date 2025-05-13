#include "Hooks/hooks.h"

#include "DialogueManager/DialogueManager.h"

namespace Hooks {
	bool Install()
	{
		SKSE::AllocTrampoline(14);
		logger::info("Installing hooks, allocated 14 bytes to the trampoline."sv);

		bool nominal = true;
		auto* dialogueItemConstructorManager = DialogueItemConstructorCall::GetSingleton();
		if (!dialogueItemConstructorManager) {
			logger::critical("  >Failed to get manager singleton for the Dialogue Item Ctor manager."sv);
			nominal = false;
		}
		auto* playerUpdateManager = PlayerUpdateListener::GetSingleton();
		if (!playerUpdateManager) {
			logger::critical("  >Failed to get manager singleton for the Player Update manager."sv);
			nominal = false;
		}
		if (!nominal) {
			return false;
		}

		logger::info("  >Installing Dialogue Item Ctor manager..."sv);
		bool installedCtorPatch = dialogueItemConstructorManager->Install();
		logger::info("  >Installing Player Update manager..."sv);
		bool installedPlayerUpdatePatch = playerUpdateManager->Install();

		return installedPlayerUpdatePatch && installedCtorPatch;
	}

	bool DialogueItemConstructorCall::Install()
	{
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> target{ REL::ID(25541), 0xE2 };
		if (!REL::make_pattern<"E8">().match(target.address())) {
			logger::critical("  >Failed to match pattern for 25541 + 0xE2."sv);
			return false;
		}
		_func = trampoline.write_call<5>(target.address(), &Thunk);
		return true;
	}

	bool PlayerUpdateListener::Install() {
		REL::Relocation<std::uintptr_t> VTABLE{ RE::PlayerCharacter::VTABLE[0] };
		_func = VTABLE.write_vfunc(idx, Thunk);
		return true;
	}

	inline void PlayerUpdateListener::Thunk(RE::PlayerCharacter* a_this, float a_delta) {
		_func(a_this, a_delta);
		internalCounter += std::max(0.0f, a_delta);
		if (internalCounter >= timeBetweenAttempts) {
			auto* manager = DialogueManager::Manager::GetSingleton();
			if (manager) {
				manager->QueueTask();
			}
			internalCounter = 0.0f;
		}
		internalCounter = std::clamp(internalCounter, 0.0f, timeBetweenAttempts);
	}

	RE::DialogueItem* DialogueItemConstructorCall::Thunk(
		RE::DialogueItem* a_this, 
		RE::TESQuest* a_quest,
		RE::TESTopic* a_topic,
		RE::TESTopicInfo* a_topicInfo,
		RE::TESObjectREFR* a_speaker)
	{
		auto* dialogueManager = DialogueManager::Manager::GetSingleton();
		auto response = _func(a_this, a_quest, a_topic, a_topicInfo, a_speaker);
		auto* speaker = a_speaker ? a_speaker->As<RE::Actor>() : nullptr;
		if (!response || 
			!dialogueManager || 
			!speaker || 
			!a_topic || 
			dialogueManager->AllowDialogue(speaker, a_topic)) 
		{
			return response;
		}
		delete a_this;
		return nullptr;
	}
}