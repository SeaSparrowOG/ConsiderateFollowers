#pragma once

namespace DialogueManager
{
	class StoredDialogue
	{
	public:
		void Process();
		bool Preserve() const;

		StoredDialogue(RE::Actor* a_actor, RE::TESTopic* a_topic);
	private:
		bool          preserve{ true };
		RE::FormID    actorFormID{ 0 };
		RE::TESTopic* topic{ nullptr };

		bool CanSpeak(RE::Character* a_storedCharacter);
	};
}