#include "DialogueManager.h"

#include "RE/Misc.h"
#include "settings/INISettings.h"

namespace DialogueManager
{
	bool Manager::AllowDialogue(RE::Actor* a_speaker, RE::TESTopic* a_topic) {
		using DialogueData = RE::DIALOGUE_DATA::Subtype;
		using DialogueType = RE::DIALOGUE_TYPE;

		if (!a_speaker || !a_topic) {
			return true;
		}

		const auto* speakerBase = a_speaker->GetActorBase();
		if (speakerBase && whitelistedNPCs.contains(speakerBase)) {
			UpdateInternalConversation(a_speaker);
			return true;
		}

		bool isFollower = a_speaker->IsPlayerTeammate();
		if (!isFollower) {
			UpdateInternalConversation(a_speaker);
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
			UpdateInternalConversation(a_speaker);
			return true;
		default:
			break;
		}

		switch (a_topic->data.type.get()) {
		case DialogueType::kSceneDialogue:
			UpdateInternalConversation(a_speaker);
			return true;
		default:
			break;
		}

		auto dialogueManager = RE::MenuTopicManager::GetSingleton();
		auto dialogueTarget = dialogueManager ? dialogueManager->speaker.get() : nullptr;
		auto* dialogueActor = dialogueTarget && dialogueTarget.get() ? dialogueTarget->As<RE::Actor>() : nullptr;
		bool isDialogueTarget = dialogueTarget ? dialogueActor == a_speaker : false;
		if ((dialogueActor && !isDialogueTarget) || IsClosestActorSpeaking()) {
			auto witheldDialogue = StoredDialogue(a_speaker, a_topic);
			if (witheldDialogue.Preserve()) {
				pendingDialogue.push_back(witheldDialogue);
			}
			return false;
		}

		UpdateInternalConversation(a_speaker);
		return true;
	}

	void Manager::UpdateInternalConversation(RE::Actor* a_speaker) {
		if (a_speaker->IsPlayerRef() || !a_speaker->Is3DLoaded()) {
			return;
		}

		if (closestSpeakerID == 0) {
			closestSpeakerID = a_speaker->GetFormID();
			return;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		auto* oldSpeaker = player ? RE::TESForm::LookupByID<RE::Character>(closestSpeakerID) : nullptr;
		if (!oldSpeaker) {
			closestSpeakerID = a_speaker->GetFormID();
			return;
		}
		else if (!oldSpeaker->Is3DLoaded() || !RE::IsTalking(oldSpeaker)) {
			closestSpeakerID = a_speaker->GetFormID();
			return;
		}
		
		auto distance = player->GetDistance(oldSpeaker);
		if (distance > maximumDistance ||
			distance > player->GetDistance(a_speaker)) {
			closestSpeakerID = a_speaker->GetFormID();
		}
	}

	bool Manager::RequestINISettings() {
		auto* iniManager = Settings::INI::Holder::GetSingleton();
		if (!iniManager) {
			logger::critical("  >Failed to get INI Manager."sv);
			return false;
		}

		auto preventPileUpRaw = iniManager->GetStoredSetting<bool>(iniManager->PREVENT_PILEUP_SETTING);
		if (!preventPileUpRaw.has_value()) {
			logger::error("  >Setting {} not found in INI settings, treating as false."sv, iniManager->PREVENT_PILEUP_SETTING);
			preventPileUp = false;
		}
		else {
			preventPileUp = preventPileUpRaw.value();
		}

		auto maxSpeakerDistanceRaw = iniManager->GetStoredSetting<float>(iniManager->MAX_DISTANCE_SETTING);
		if (!maxSpeakerDistanceRaw.has_value()) {
			logger::error("  >Setting {} not found in INI settings, treating as 500.0."sv, iniManager->MAX_DISTANCE_SETTING);
			maximumDistance = 500.0f;
		}
		else {
			maximumDistance = std::clamp<float>(maxSpeakerDistanceRaw.value(), 0.0f, 2500.0f);
		}
		return true;
	}

	void Manager::RegisterWhitelistedNPCs(const std::vector<const RE::TESNPC*>& a_newNPCs)
	{
		for (const auto* npc : a_newNPCs) {
			if (!npc || whitelistedNPCs.contains(npc)) {
				continue;
			}
			whitelistedNPCs.insert(npc);
		}
	}

	void Manager::Run() {
		if (pendingDialogue.empty()) {
			return;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		bool playerBusy = player ? RE::PlayerIsSleepingOrResting(player) : true;
		if (playerBusy || IsClosestActorSpeaking()) {
			return;
		}

		auto dialogueManager = RE::MenuTopicManager::GetSingleton();
		auto dialogueTarget = dialogueManager ? dialogueManager->speaker.get() : nullptr;
		auto* dialogueActor = dialogueTarget && dialogueTarget.get() ? dialogueTarget->As<RE::Actor>() : nullptr;
		if (dialogueActor || IsClosestActorSpeaking()) {
			return;
		}
		pendingDialogue.back().Process();
	}

	void Manager::Dispose() {
		if (pendingDialogue.empty()) {
			queued = false;
			return;
		}

		auto newStoredDialogue = std::vector<StoredDialogue>();
		newStoredDialogue.reserve(pendingDialogue.size());

		for (auto& heldDialogue : pendingDialogue) {
			if (!heldDialogue.Preserve()) {
				continue;
			}
			newStoredDialogue.push_back(std::move(heldDialogue));
		}
		pendingDialogue.clear();
		pendingDialogue = newStoredDialogue;
		queued = false;
	}

	void Manager::QueueTask() {
		if (queued) {
			return;
		}

		auto* interface = SKSE::GetTaskInterface();
		auto* tasklet = reinterpret_cast<::TaskDelegate*>(this);
		if (!interface || !tasklet) {
			return;
		}
		queued = true;
		interface->AddTask(tasklet);
	}

	bool Manager::IsClosestActorSpeaking() const {
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player || closestSpeakerID == 0) {
			return false;
		}

		auto* closestSpeaker = RE::TESForm::LookupByID<RE::Character>(closestSpeakerID);
		if (!closestSpeaker || !closestSpeaker->Is3DLoaded()) {
			return false;
		}

		bool talking = RE::IsTalking(closestSpeaker);
		float speakerDistance = talking ? closestSpeaker->GetDistance(player) : 5000.0f;
		if (speakerDistance > maximumDistance) {
			return false;
		}
		return true;
	}
}