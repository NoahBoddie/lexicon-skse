#pragma once

#include"FunctorManager.h"


namespace LEX
{
	//If I'm being honest, this is preferable, as it will only strike once.
	struct FilesLoadedHook
	{
		static void Install()
		{
			//SE: 16DBB0, AE(6.640): 179E50, VR : ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID { 13639, 13744 } , RELOCATION_OFFSET(0x2FA, 0x30E)};

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address(), thunk);
		}

		static void thunk(uintptr_t a1)
		{
			func(a1);

			//Component::LinkComponents(LinkFlag::External);
			FunctorManager::Initialize();
			Component::LinkComponents(LinkFlag::External);

		}

		inline static REL::Relocation<decltype(thunk)> func;
	};

}