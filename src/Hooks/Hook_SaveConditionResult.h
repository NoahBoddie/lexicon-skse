#pragma once

#include "ConditionTLS.h"

namespace LEX
{
	struct SaveConditionResultHook
	{
		static void Install()
		{
			//SE: 4454C0, AE(6.640): 460B30, VR : ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID { 29090, 29924, 29090 } , RELOCATION_OFFSET(0x6D8, 0x6FC) };

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address(), thunk);
		}

		static bool thunk(int32_t a1, float a2, float a3)
		{
			auto result = func(a1, a2, a3);

			if (currentParams && currentParams->ShouldStoreSolution() == true)
				currentParams->prevSolution = a2;

			return result;
		}

		inline static REL::Relocation<decltype(thunk)> func;
	};

}