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
				logger::debug("Processing...");
				const auto speakerCharacter = speaker->As<RE::Character>();
				const auto speakerActor = speaker->As<RE::Actor>();
				if (RE::IsTalking(speakerCharacter)) {
					return kSkip;
				}

				logger::debug("Accepted!");
				Papyrus::EventDispatcher::GetSingleton()->followerShouldCommentate.QueueEvent(speakerActor, topic);
				return kCompleted;
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
			internalCounter += a_delta;

			if (internalCounter >= 3.0f) {
				DialogueItemConstructorCall::GetSingleton()->ReleaseDialogueIfPossible();
				internalCounter = 0.0f;
			}
		}

		inline static REL::Relocation<decltype(UpdateVFunc::thunk)> func;
		static constexpr std::size_t idx{ 0xAD }; //Update

		inline static float internalCounter{ 0.0f };

	};
}