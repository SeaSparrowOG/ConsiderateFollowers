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
		const auto response = _createDialogueItem(a_this, a_quest, a_topic, a_topicInfo, a_speaker);

		if (a_speaker && a_speaker->As<RE::Actor>()) {
			const auto speakerActor = a_speaker->As<RE::Actor>();
			if (!speakerActor) {
				return response;
			}

			const auto singleton = DialogueItemConstructorCall::GetSingleton();
			assert(singleton);
			singleton->UpdateInternalClosestConversation(speakerActor);

			if (speakerActor->IsPlayerTeammate() && singleton->ShouldFollowerRespond(speakerActor, a_topic)) {
				return response;
			}

			delete a_this; //is this needed?
			return nullptr;
		}
		return response;
	}

	bool DialogueItemConstructorCall::ShouldFollowerRespond(RE::Actor* a_speaker, RE::TESTopic* a_topic)
	{
		using DialogueData = RE::DIALOGUE_DATA::Subtype;
		using DialogueType = RE::DIALOGUE_TYPE;
		bool talkingToPlayer = a_speaker->GetHighProcess() ? a_speaker->GetHighProcess()->talkingToPC : false;
		if (talkingToPlayer) {
			return true;
		}

		switch (a_topic->data.subtype.get()) {
		case DialogueData::kAlertIdle:
		case DialogueData::kAlertToCombat:
		case DialogueData::kBash:
		case DialogueData::kCombatGrunt:
		case DialogueData::kDeath:
		case DialogueData::kForceGreet:
		case DialogueData::kHit:
		case DialogueData::kLostToCombat:
		case DialogueData::kNormalToCombat:
		case DialogueData::kVoicePowerEndLong:
		case DialogueData::kVoicePowerStartLong:
		case DialogueData::kVoicePowerEndShort:
		case DialogueData::kVoicePowerStartShort:
			return true;
		default:
			break;
		}

		switch (a_topic->data.type.get()) {
		case DialogueType::kSceneDialogue:
			return true;
		default:
			break;
		}

		const auto menuTopicManager = RE::MenuTopicManager::GetSingleton();
		assert(menuTopicManager);
		const auto currentPlayerDialogueTarget = menuTopicManager->speaker.get().get();
		if (currentPlayerDialogueTarget && currentPlayerDialogueTarget == a_speaker) {
			return true;
		}

		if (closestSpeaker && closestSpeaker->Is3DLoaded()) {
			const auto player = RE::PlayerCharacter::GetSingleton();
			assert(player);
			const auto distance = player->GetDistance(closestSpeaker);
			if (distance > maximumDistance) {
				return true;
			}
		}
		return false;
	}

	void DialogueItemConstructorCall::UpdateInternalClosestConversation(RE::Actor* a_speaker)
	{
		if (!a_speaker->IsPlayerTeammate()) {
			const auto player = RE::PlayerCharacter::GetSingleton();
			assert(player);
			bool shouldReplace = false;
			if (!closestSpeaker || !closestSpeaker->Is3DLoaded()) {
				shouldReplace = true;
			}
			if (!shouldReplace) {
				const auto newDistance = player->GetDistance(a_speaker);
				const auto oldDistance = player->GetDistance(closestSpeaker);
				if (newDistance < oldDistance) {
					shouldReplace = true;
				}
			}

			if (shouldReplace) {
				closestSpeaker = a_speaker;
			}
		}
	}

	void Install()
	{
		DialogueItemConstructorCall::GetSingleton()->Install();
	}
}