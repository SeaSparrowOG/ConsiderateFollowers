#include "StoredDialogue.h"

#include "RE/Misc.h"

namespace DialogueManager
{
	bool StoredDialogue::CanSpeak(RE::Character* a_storedCharacter) {
		if (!a_storedCharacter->IsPlayerTeammate()) {
			preserve = false;
		}

		auto lifeState = a_storedCharacter->actorState1.lifeState;
		switch (lifeState) {
		case RE::ACTOR_LIFE_STATE::kDying:
		case RE::ACTOR_LIFE_STATE::kDead:
			preserve = false;
			return false;
		case RE::ACTOR_LIFE_STATE::kReanimate:
			preserve = false;
			return true;
		default:
			break;
		}

		auto process = a_storedCharacter->currentProcess;
		if (!process || 
			RE::GetCharacterProcessLevel(a_storedCharacter) == RE::PROCESS_TYPE::kNone)
		{
			preserve = false;
			return false;
		}

		if (!a_storedCharacter->Is3DLoaded()) {
			return false;
		}

		return true;
	}

	void StoredDialogue::Process() {
		auto* internalCharacter = RE::TESForm::LookupByID<RE::Character>(actorFormID);
		if (!internalCharacter || !topic) {
			preserve = false;
			return;
		}
		if (!CanSpeak(internalCharacter)) {
			return;
		}

		auto newSoundHandle = RE::BSSoundHandle();
		newSoundHandle.assumeSuccess = false;
		newSoundHandle.soundID = RE::BSSoundHandle::kInvalidID;
		newSoundHandle.state = RE::BSSoundHandle::AssumedState::kInitialized;

		auto* player = RE::PlayerCharacter::GetSingleton();
		bool playerResting = player ? RE::PlayerIsSleepingOrResting(player) : true;
		if (playerResting) {
			std::unique_ptr<RE::DialogueItem> dialogueItem(RE::CreateDialogueItem(topic, internalCharacter));
			if (dialogueItem) {
				RE::DialogueItemFirstResponse(dialogueItem.get());
				RE::RunResult(dialogueItem.get(), 0, false);
				RE::RunResult(dialogueItem.get(), 1, false);
			}
		}
		else {
			RE::AIProcessProccessGreet(internalCharacter->currentProcess, internalCharacter, topic);
		}
		preserve = false;
	}

	bool StoredDialogue::Preserve() const {
		return preserve;
	}

	StoredDialogue::StoredDialogue(RE::Actor* a_actor, RE::TESTopic* a_topic)
	{
		if (a_actor && a_topic) {
			this->actorFormID = a_actor->GetFormID();
			this->topic = a_topic;
			this->preserve = true;
		}
		else {
			this->actorFormID = 0;
			this->topic = nullptr;
			this->preserve = false;
		}
	}
}
