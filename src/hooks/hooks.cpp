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
		using DialogueData = RE::DIALOGUE_DATA::Subtype;
		using DialogueType = RE::DIALOGUE_TYPE;

		const auto response = _createDialogueItem(a_this, a_quest, a_topic, a_topicInfo, a_speaker);
		if (a_speaker) {
			if (const auto speakerActor = a_speaker->As<RE::Actor>();
				speakerActor && speakerActor->IsPlayerTeammate()) {
				logger::debug("Player teammate {} tried to talk...", speakerActor->GetName());
				bool talkingToPlayer = speakerActor->GetHighProcess() ? speakerActor->GetHighProcess()->talkingToPC : false;
				if (talkingToPlayer) {
					logger::debug("  ... and was talking to the player.");
					return response;
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
					logger::debug("  ... but subtype was invalid.");
					return response;
				default:
					break;
				}

				switch (a_topic->data.type.get()) {
				case DialogueType::kSceneDialogue:
					logger::debug("  ... but type was invalid.");
					return response;
				default:
					break;
				}

				const auto menuTopicManager = RE::MenuTopicManager::GetSingleton();
				assert(menuTopicManager);
				const auto currentPlayerDialogueTarget = menuTopicManager->speaker.get().get();
				if (currentPlayerDialogueTarget && currentPlayerDialogueTarget != speakerActor) {
					logger::debug("   ... but player was in dialogue with {}.", currentPlayerDialogueTarget->GetName());
					delete a_this;
					return nullptr;
				}
				logger::debug("  ... and finished their sentence.");
			}
		}
		return response;
	}

	void Install()
	{
		DialogueItemConstructorCall::GetSingleton()->Install();
	}
}