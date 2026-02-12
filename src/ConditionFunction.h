#pragma once

#include "xbyak/xbyak.h"
#include "Console.h"
#include "FunctorManager.h"

#include "ConditionTLS.h"

namespace RE
{
	using FunctionID = FUNCTION_DATA::FunctionID;
}

namespace LEX
{
	constexpr std::string_view init_chars = "RUN";

	constexpr auto init_size = init_chars.size();


	int CallOrJump(uintptr_t addr)
	{
		//0x15 0xE8//These are calls, represented by negative numbers
		//0x25 0xE9//These are jumps, represented by positive numbers.
		//And zero represent it being neither.

		if (addr)
		{
			auto first_byte = reinterpret_cast<uint8_t*>(addr);

			switch (*first_byte)
			{
			case 0x15:
			case 0xE8:
				return -1;

			case 0x25:
			case 0xE9:
				return 1;

			}
		}

		return 0;
	}



	using ConditionFormula = Formula<double(RE::TESObjectREFR::*)(RE::TESObjectREFR*, RE::TESObjectREFR*)>;
	using ConsoleFormula = Formula<Voidable(RE::TESObjectREFR::*)(Voidable)>;


	struct ConditionHook
	{
		///SkyrimSE.exe+4442B0+87 is actually where we'd want to set up the formulas.

		
		static void Install()
		{
			auto script = RE::SCRIPT_FUNCTION::LocateScriptCommand("GetNoRumors");

			func = script->conditionFunction;

			script->conditionFunction = thunk;


			logger::info("ConD1Hook complete...");

			/*
			return;

			//should do this to the float version as well.
			auto script = RE::SCRIPT_FUNCTION::LocateScriptCommand("GetGraphVariableInt");

			func = script->conditionFunction;

			script->conditionFunction = thunk;

			
			logger::info("ConD1Hook complete...");
			//*/
		}

		//A seperate hook will have to be established to turn off the condition function check.

		static bool thunk(RE::TESObjectREFR* a_this, void* param1, void* param2, double& result)
		{

			switch (reinterpret_cast<size_t>(param2))
			{
			case 0xDEADBEEF:
				if  constexpr (1)
				{
					auto& formula = reinterpret_cast<ConditionFormula&>(param1);

					RE::TESObjectREFR* subject;
					RE::TESObjectREFR* target;

					if (auto params = currentParams->GetParams()) {
						subject = params->actionRef;
						target = params->targetRef;
					}
					else {
						subject = target = nullptr;
					}

					result = formula ? formula(a_this)->Call(subject, target, NAN) : NAN;
				}
				return true;

			case FunctorManager::k_exFuncCode:
				if  constexpr (1)
				{
					Functor* functor = reinterpret_cast<Functor*>(param1);
					result = functor->Execute(a_this);
				}
				return true;

			case FunctorManager::k_loadPropCode:
				if  constexpr (1)
				{
					Property* property = reinterpret_cast<Property*>(param1);
					result = property->LoadAsArgument();
				}
				return true;

			case FunctorManager::k_loadFileCode:
				result = !!param1 ? 1.0 : -1.0;
				return true;

			default:
				return func(a_this, param1, param2, result);

			}

		}


		static inline RE::SCRIPT_FUNCTION::Condition_t* func = nullptr;
	};


	struct ConsoleHook
	{
		static void Install()
		{
			//SE: 8DAE20, AE: 91CC70, VR: ???
			REL::RelocationID hook { 52065, 52952, 52065 };
			ptrdiff_t offset = REL::VariantOffset(0xE2, 0x52, 0xE2).offset();


			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address() + offset, thunk);


			logger::info("ConD2Hook complete...");
			//*/
		}

		//A seperate hook will have to be established to turn off the condition function check.

		static bool thunk(RE::Script* a_this, RE::ScriptCompiler* a2, RE::COMPILER_NAME a3, RE::TESObjectREFR* a4)
		{
			

			std::string_view text = a_this->text;

		

			//So here's what I want in the strings for these.
			//Success message, failure message, completion message. I'll use strings cause these are some what personalized.

			if (auto str = a_this->text; !strnicmp(str, init_chars.data(), init_size) && text.size() > init_size && std::isalnum(str[init_size]) == false)
			{
				//for now, I'm skipping everything until I get to a : character.

				size_t length = text.size() - init_size;

				auto begin = str + init_size;

				//This needs to be a unique character, this cannot suffice. 
				// some unique string of characters that's immistakable, or use the parser. That's also viable.
				while (*begin != '\0' && strnicmp(begin, ">>", 2) != 0) length--, begin++;


				if (*begin != '\0')
				{
					length--, begin++;
					length--, begin++;



					std::string_view formula{ begin, length };

					report::apply::debug("running '{}'", formula);
					
					auto log = RE::ConsoleLog::GetSingleton();

					try
					{

						auto func = ConsoleFormula::Create("previous", formula);
						previousResult = func(a4)->Call(previousResult);

						//Variable result = Formula<Variable>::Run(formula);

						auto log = RE::ConsoleLog::GetSingleton();
						 
						if (previousResult.IsVoid() == false)
							log->Print("result >> %s", previousResult.PrintString().c_str());

						return true;
					}
					catch (const LEX::Error& error)
					{
						log->Print("Script compiling error, adjust syntax (temp message)");
						return true;
					}

				}
				else
				{
					auto log = RE::ConsoleLog::GetSingleton();

					log->Print("No script content detected.");
				}
			}


			return func(a_this, a2, a3, a4);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};



	struct DeleteConditionHook
	{
		static void Install()
		{
			//SE: 444B80, AE(6.640): 460220, VR : ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID { 29079, 29896, 29079 } , 0x25 };

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address(), thunk);
		}

		static uint32_t thunk(RE::FUNCTION_DATA* a_this)
		{
			auto result = func(a_this);

			if (a_this->function == RE::FunctionID::kGetNoRumors) {
				if (reinterpret_cast<size_t>(a_this->params[1]) != 0xDEADBEEF) {
					reinterpret_cast<ConditionFormula&>(a_this->params[0]).~ConditionFormula();
				}
			}

			return result;
		}

		inline static REL::Relocation<decltype(thunk)> func;
	};


	struct LoadConditionHook
	{
		static void Install()
		{
			//SE: 4442B0, AE(6.640): 45F890, VR : ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID { 29064, 29876 } , 0x87 };

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address(), thunk);
		}

		static void thunk(RE::TESConditionItem* a_this, RE::TESFile* file)
		{
			constexpr auto replacement = RE::FunctionID::kGetNoRumors;

			func(a_this, file);

			auto& func_data = a_this->data.functionData;
			auto& func_id = a_this->data.functionData.function;
			
			switch (*func_id)
			{
			case RE::FunctionID::kGetGraphVariableInt:
			case RE::FunctionID::kGetGraphVariableFloat:
				if (func_data.params[0])
					break;

			default: 
				return;
			}
			
			if constexpr (0)
			if (static bool once = true; std::exchange(once, false)) {
				RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

				if (!handler) {
					logger::warn("not yet");
				}

				std::span<RE::TESFile*> heavies{ handler->GetLoadedLightMods(), handler->GetLoadedLightModCount() };
				std::span<RE::TESFile*> lights{ handler->GetLoadedMods(), handler->GetLoadedModCount() };

				logger::info("Heavies:");
				for (auto file : heavies)
				{
					logger::info("	{}", file->GetFilename());
				}
				logger::info("Lights:");
				for (auto file : lights)
				{
					logger::info("	{}", file->GetFilename());
				}

			}


			RE::BSFixedString*& arg = reinterpret_cast<RE::BSFixedString*&>(func_data.params[0]);
			//arg.clear
			if (auto str = arg->c_str(); !strnicmp(str, init_chars.data(), init_size) && arg->size() > init_size && std::isalnum(str[init_size]) == false)
			{
				//for now, I'm skipping everything until I get to a : character.

				size_t length = arg->size() - init_size;

				auto begin = str + init_size;

				//This needs to be a unique character, this cannot suffice. 
				// some unique string of characters that's immistakable, or use the parser. That's also viable.
				while (*begin != '\0' && strnicmp(begin, ">>", 2) != 0) length--, begin++;


				if (*begin != '\0')
				{
					auto string = arg;

					func_data.function = RE::FunctionID::kGetNoRumors;
					func_data.params[0] = nullptr;
					reinterpret_cast<size_t&>(func_data.params[1]) = 0xDEADBEEF;


					length--, begin++;
					length--, begin++;



					std::string_view form{ begin, length };

					report::compile::info("compiling '{}'", form);

					ConditionFormula formula = ConditionFormula::Create("subject", "target", form);

					if (formula) {
						reinterpret_cast<ConditionFormula&>(arg) = std::move(formula);
						report::compile::info("Successfully compiled '{}'", form);
					}
					else {
						report::compile::failure("Condition [{}] failed to compile.", form);
					}

					string->~BSFixedString();

				}
			}

		}

		inline static REL::Relocation<decltype(thunk)> func;
	};



	//If I'm being honest, this is preferable, as it will only strike once.
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




	//If I'm being honest, this is preferable, as it will only strike once.
	struct LoadConditionFinalHook
	{
		static void Install()
		{
			//SE: 444370, AE(6.640): 45F960, VR : ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID { 29065, 29877 } , 0x1B };

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address(), thunk);
		}

		static void thunk(RE::TESConditionItem* a_this, RE::TESForm* form)
		{
			constexpr auto replacement = RE::FunctionID::kGetNoRumors;

			func(a_this, form);

			auto& func_data = a_this->data.functionData;
			auto& func_id = func_data.function;

			switch (*func_id)
			{
			//case RE::FunctionID::kGetGraphVariableInt:
			//case RE::FunctionID::kGetGraphVariableFloat:
			//	if (func_data.params[0])
			//		break;

			case RE::FunctionID::kHasKeyword: {
				RE::BGSKeyword* keyword = reinterpret_cast<RE::BGSKeyword*>(func_data.params[0]);
				if (FunctorManager::Apply(keyword, func_data.params[0], func_data.params[1]) == true) {
					func_id = replacement;
				}
				return;
			}

			default:
				return;
			}

		}

		inline static REL::Relocation<decltype(thunk)> func;
	};




	struct SaveConditionParamsHook
	{

		static void Install()
		{
			//SE: 4454C0, AE: 460B30, VR: ???
			auto hook = REL::RelocationID(29090, 29924).address();
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


			logger::info("SaveConditionParamsHook complete...");
			//*/
		}

		static bool thunk(RE::TESConditionItem* a_this, RE::ConditionCheckParams* params)
		{
			ConditionTLS storage{ params };

			auto old = currentParams;
			currentParams = &storage;

			auto result = func(a_this, params);

			
			currentParams = old;

			return result;
		}

		inline static REL::Relocation<decltype(thunk)> func;
	};


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


	void Install()
	{
		SKSE::AllocTrampoline(14 * 7);

		//SE: 328110 + 75F//This can be used to get the result of a given console command call. I can use this to get the last value of the console
		// then load it into a thread local system or something like that, allowing it to return a value through a reference.
		SaveConsoleResultHook::Install();
		SaveConditionParamsHook::Install();
		DeleteConditionHook::Install();
		LoadConditionHook::Install();
		LoadConditionFinalHook::Install();
		ResolveConditionHook::Install();//Exists
		ConditionHook::Install();
		ConsoleHook::Install();
	}

}