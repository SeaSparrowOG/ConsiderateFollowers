#include "papyrus.h"

namespace Papyrus
{
	std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	bool RegisterForAllEvents(STATIC_ARGS, RE::TESForm* a_form) {
		if (!a_form) {
			return false;
		}
		EventDispatcher::GetSingleton()->AddListener(a_form);
		logger::debug("Registered form for events");
		return true;
	}

	void Bind(VM& a_vm) {
		BIND(GetVersion);
		BIND_EVENT(RegisterForAllEvents, true);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}