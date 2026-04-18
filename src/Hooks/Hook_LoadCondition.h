#pragma once

#include "FunctorManager.h"

namespace LEX
{

	//If I'm being honest, this is preferable, as it will only strike once.
	struct LoadConditionHook
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
			case RE::FunctionID::kGetGraphVariableInt:
			case RE::FunctionID::kGetGraphVariableFloat:
				if constexpr (1)
				{
					if (!func_data.params[0])
						break;

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
				break;

			case RE::FunctionID::kHasKeyword: {
				RE::BGSKeyword* keyword = reinterpret_cast<RE::BGSKeyword*>(func_data.params[0]);

				if (FunctorManager::Apply(keyword, func_data.params[0], func_data.params[1], a_this->data) == true) {
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

}