#pragma once

#include "ConditionTLS.h"
#include "xbyak/xbyak.h"


namespace LEX
{
	struct ResolveConditionHook
	{

		static void Install()
		{
			//SE: 444370, AE(6.640): 45F960, VR : ???
			auto hook = REL::RelocationID(29065, 29877).address();
			uintptr_t offset = 0x5;


			struct Patch : Xbyak::CodeGenerator
			{
				explicit Patch(uintptr_t address, uintptr_t length)
				{
					// Hook returns here. Execute the restored bytes and jump back to the original function.
					for (size_t i = 0; i < length; i++)
						db(*reinterpret_cast<uint8_t*>(address + i));

					jmp(ptr[rip]);
					dq(address + length);
				}
			} static code{ hook, offset };



			auto& trampoline = SKSE::GetTrampoline();

			//func = (uintptr_t)code.getCode();

			//trampoline.write_branch<5>(hook_addr, thunk);

			//return;

			auto placed_call = CallOrJump(hook) > 0;

			auto place_query = trampoline.write_branch<5>(hook, (uintptr_t)thunk);

			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;


			logger::info("ResolveConditionHook complete...");
			//*/
		}



		static void thunk(RE::TESCondition* a_this, RE::TESForm* form)
		{

			ConditionTLS storage{};

			auto old = currentParams;
			currentParams = &storage;
			func(a_this, form);
			currentParams = old;


		}

		inline static REL::Relocation<decltype(thunk)> func;
	};
}