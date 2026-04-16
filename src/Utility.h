#pragma once

#include "xbyak/xbyak.h"
#include "Zydis/Decoder.h"

namespace LEX
{
	struct ProloguePatch : Xbyak::CodeGenerator
	{
		inline static ZydisDecoder decoder{};

		explicit ProloguePatch(uintptr_t address, uintptr_t length)
		{
			
			{
				ZydisDecodedInstruction instr;

				auto status = ZydisDecoderDecodeInstruction(&decoder, nullptr, reinterpret_cast<const void*>(address), length, &instr);

				assert_if(!ZYAN_SUCCESS(status)) {
					//Some kind of failure.
				}

				length = instr.length;
			}

			// Hook returns here. Execute the restored bytes and jump back to the original function.
			for (size_t i = 0; i < length; i++)
				db(*reinterpret_cast<uint8_t*>(address + i));

			jmp(ptr[rip]);
			dq(address + length);
		}

		static void* GetInstructions(SKSE::Trampoline& trampoline, uintptr_t address, uintptr_t length)
		{
			ProloguePatch it{ address, length };

			return it.GetInstructions(trampoline);
		}

		static void* GetInstructions(uintptr_t address, uintptr_t length)
		{
			auto& trampoline = SKSE::GetTrampoline();
			return GetInstructions(trampoline, address, length);
		}

		void* GetInstructions(SKSE::Trampoline& trampoline)
		{
			ready();

			auto size = getSize();

			auto& trampoline = SKSE::GetTrampoline();

			auto alloc = trampoline.allocate(size);
			std::memcpy(alloc, getCode(), size);
			return alloc;
		}


	};
}