#pragma once

#include "utilities/utilities.h"

namespace Papyrus {
#define BIND(a_method, ...) a_vm.RegisterFunction(#a_method##sv, script, a_method __VA_OPT__(, ) __VA_ARGS__)
#define BIND_EVENT(a_method, ...) a_vm.RegisterFunction(#a_method##sv, script, a_method __VA_OPT__(, ) __VA_ARGS__)
#define STATIC_ARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] StackID a_stackID, RE::StaticFunctionTag *

	using VM = RE::BSScript::Internal::VirtualMachine;
	using StackID = RE::VMStackID;
	inline auto script = fmt::format("SEA_{}"sv, Plugin::NAME);
	bool RegisterFunctions(VM* a_vm);

	class EventDispatcher : public Utilities::Singleton::ISingleton<EventDispatcher> {
	public:
		void AddListener(const RE::TESForm* a_form) {
			followerShouldCommentate.Register(a_form);
		}

		SKSE::RegistrationSet<RE::Actor*, RE::TESTopic*> followerShouldCommentate{ "OnFollowerShouldComment"sv };
	};
}