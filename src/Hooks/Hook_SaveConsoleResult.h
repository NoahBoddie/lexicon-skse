#pragma once

#include "xbyak/xbyak.h"
#include "Console.h"

namespace LEX
{

	struct SaveConsoleResultHook
	{

		static void Install()
		{
			//SE: 328110, AE: 33F730, VR: ???
			auto hook = REL::Relocation<uintptr_t>{ REL::RelocationID(22879, 23329), RELOCATION_OFFSET(0x75F, 0x771) }.address();
			uintptr_t offset = 0x5;


			struct Patch : Xbyak::CodeGenerator
			{
				explicit Patch(uintptr_t address, uintptr_t func)
				{
					//movups(xmm2, ptr[rsp + 0x40]);
					//movsd(xmm0, ptr[rsp + 0x40]);
					//movups(xmm1, ptr[rsp + 0x40]);

					mov(rax, std::bit_cast<uint64_t>(noReturn));
					mov(ptr[rsp + 0x40], rax);
					//mov(rax, ptr[rsp + 0x30]);
					//mov(ptr[rsp + 0x48], rax);

					call(r10);
					mov(cl, al);
					movups(xmm1, ptr[rsp + 0x40]);
					//mov(r8, ptr[rsp + 0x30]);

					mov(rax, func);
					call(rax);
					test(al, al);

					jmp(ptr[rip]);
					dq(address + 0x5);

					/*

					mov(rax, ptr[rsp + 0x30]);
					mov(ptr[rsp + 0x48], rax);

					call(r10);
					mov(cl, al);
					mov(rdx, ptr[rsp + 0x48]);
					mov(r8, ptr[rsp + 0x30]);

					mov(rax, func);
					call(rax);
					test(al, al);

					jmp(ptr[rip]);
					dq(address + 0x5);


					//*/


					//return;
					//mov(ptr[rsp + 0x48], r10);
					//mov(rax, func);
					//call(rax);


				}
			} static code{ hook, (uintptr_t)thunk };



			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook, code.getCode());

			logger::info("SaveConsoleResultHook complete...");
			//*/
		}
		using Func = bool(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, double&, uint64_t);

		static bool thunk(bool result, double after)
		{
			//auto old = a7;
			//auto result = func(a1, a2, a3, a4, a5, a6, a7, a8);

			if (result) {
				consoleReturn = after;
			}
			report::message::trace("before?: {}, after: {}", noReturn, after);
			return result;
		}

	};

}