#pragma once

#include "Console.h"
#include "FunctorManager.h"
namespace LEX
{


	struct RunConsoleHook
	{
		static void Install()
		{
			//SE: 8DAE20, AE: 91CC70, VR: 90E1F0
			REL::RelocationID hook{ 52065, 52952 };
			ptrdiff_t offset = RELOCATION_OFFSET(0xE2, 0x52);


			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address() + offset, thunk);


			logger::info("ConD2Hook complete...");
			//*/
		}

		//A seperate hook will have to be established to turn off the condition function check.

		static void thunk(RE::Script* a1, RE::ScriptCompiler* a2, RE::COMPILER_NAME a3, RE::TESObjectREFR* a4, void* a5)
		{

			std::string_view text = a1->text;



			//So here's what I want in the strings for these.
			//Success message, failure message, completion message. I'll use strings cause these are some what personalized.

			if (auto str = a1->text; !strnicmp(str, init_chars.data(), init_size) && text.size() > init_size && std::isalnum(str[init_size]) == false)
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
					}
					catch (const LEX::Error& error)
					{
						log->Print("Script compiling error, adjust syntax (temp message)");
					}

					return;
				}
				else
				{
					auto log = RE::ConsoleLog::GetSingleton();

					log->Print("No script content detected.");
				}
			}


			return func(a1, a2, a3, a4, a5);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

}