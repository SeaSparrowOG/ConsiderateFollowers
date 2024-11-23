#pragma once

#include "RE/Misc.h"
#include "papyrus/papyrus.h"
#include "utilities/utilities.h"

namespace Hooks {
	void Install();

	class DialogueItemConstructorCall :
		public Utilities::Singleton::ISingleton<DialogueItemConstructorCall> {
	public:
		void Install();
		void SetMaxDistance(double a_newDistance);
		void SetFollowerPileUp(bool a_pileup);
		void RegisterWhitelistedActor(const RE::TESNPC* a_actor);
		void RegisterWhitelistedQuest(const RE::TESQuest* a_quest);

		bool ReleaseDialogueIfPossible();
		bool IsClosestActorSpeaking();

	private:
		enum PendingDialogueResponse {
			kSkip,
			kCompleted
		};

		struct PendingDialogue {
			RE::TESObjectREFR* speaker;
			RE::TESTopic*      topic;

			PendingDialogue(RE::TESObjectREFR* a_speaker, RE::TESTopic* a_topic) {
				this->speaker = a_speaker;
				this->topic = a_topic;
			}

			PendingDialogueResponse Process() {
				const auto speakerCharacter = speaker->As<RE::Character>();
				if (RE::IsTalking(speakerCharacter)) {
					return kSkip;
				}
				auto newSoundHandle = RE::BSSoundHandle();
				newSoundHandle.assumeSuccess = false;
				newSoundHandle.soundID = RE::BSSoundHandle::kInvalidID;
				newSoundHandle.state = RE::BSSoundHandle::AssumedState::kInitialized;
				const auto player = RE::PlayerCharacter::GetSingleton();
				assert(player);

				const auto isSleepingOrResting = RE::PlayerIsSleepingOrResting(player);
				const auto speakerCurrentProcess = speakerCharacter->currentProcess ? RE::GetCharacterProcessLevel(speakerCharacter) : RE::PROCESS_TYPE::kNone;

				if (!isSleepingOrResting && speakerCurrentProcess == RE::PROCESS_TYPE::kHigh) {
					const auto handle = speakerCharacter->currentProcess->GetHeadtrackTarget();
					const auto target = handle.get();
					const auto targetActor = target.get() ? target.get()->As<RE::Actor>() : nullptr;

					if (targetActor) {
						speakerCharacter->currentProcess->SetHeadtrackTarget(targetActor, targetActor->editorLocCoord);
					}
					speakerCharacter->currentProcess->ClearActionHeadtrackTarget(false);
					RE::ClearGreetingInfoData(speakerCharacter->currentProcess);

					if (RE::PlayerIsSleepingOrResting(player) ||
						speakerCurrentProcess != RE::PROCESS_TYPE::kHigh ||
						(speakerCharacter->IsEssential() && speakerCharacter->GetLifeState() == RE::ACTOR_LIFE_STATE::kEssentialDown))
					{
						std::unique_ptr<RE::DialogueItem> dialogueItem(RE::CreateDialogueItem(topic, speaker));
						if (dialogueItem) {
							RE::DialogueItemFirstResponse(dialogueItem.get());
							RE::RunResult(dialogueItem.get(), 0, false);
							RE::RunResult(dialogueItem.get(), 1, false);
						}
					}
					else {
						RE::AIProcessProccessGreet(speakerCharacter->currentProcess, speakerCharacter, topic);
					}
					return kCompleted;
				}

				return kSkip;
			}

			bool HasValidData() {
				const auto speakerCharacter = speaker->As<RE::Character>();
				if (!speakerCharacter) {
					return false;
				}
				if (!speakerCharacter->Is3DLoaded() || !speakerCharacter->IsPlayerTeammate()) {
					return false;
				}
				return true;
			}
		};

		static RE::DialogueItem* CreateDialogueItem(
			RE::DialogueItem* a_dialogueItem,
			RE::TESQuest* a_quest,
			RE::TESTopic* a_topic,
			RE::TESTopicInfo* a_topicInfo,
			RE::TESObjectREFR* a_speaker);

		bool ShouldFollowerRespond(RE::Actor* a_speaker, RE::TESTopic* a_topic);

		// This isn't perfect, but is better than closestConversation. The limitation is simple:
		// Actor A starts talking, is closest.
		// Actor B starts talking, is NOT closest.
		// Player moves closer to Actor B before Actor B speaks again, and internally we will
		// test against Actor A instead of B.
		void UpdateInternalClosestConversation(RE::Actor* a_speaker);

		inline static REL::Relocation<decltype(&CreateDialogueItem)> _createDialogueItem;

		RE::Actor* closestSpeaker{ nullptr };
		float maximumDistance{ 500.0f };
		bool preventFollowerPileup{ false };
		std::vector<const RE::TESNPC*> whitelistedActors;
		std::vector<const RE::TESQuest*> whitelistedQuests;
		std::vector<PendingDialogue> queuedLines;
	};

	struct UpdateVFunc {
		static void Install() {
			stl::write_vfunc<RE::PlayerCharacter, UpdateVFunc>();
		}

		static void thunk(RE::PlayerCharacter* a_this, float a_delta) {
			func(a_this, a_delta);
			const auto ui = RE::UI::GetSingleton();
			const auto singleton = DialogueItemConstructorCall::GetSingleton();
			if (ui && singleton) {
				if (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) || singleton->IsClosestActorSpeaking()) {
					internalCounter = 0.0f;
				}
				else {
					internalCounter += a_delta;

					if (internalCounter >= 3.0f) {
						DialogueItemConstructorCall::GetSingleton()->ReleaseDialogueIfPossible();
						internalCounter = 0.0f;
					}
				}
			}
		}

		inline static REL::Relocation<decltype(UpdateVFunc::thunk)> func;
		static constexpr std::size_t idx{ 0xAD }; //Update

		inline static float internalCounter{ 0.0f };

	};
}