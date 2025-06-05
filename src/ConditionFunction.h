#pragma once


namespace LEX
{
	constexpr std::string_view init_chars = "RUN";

	constexpr auto init_size = init_chars.size();

	struct ConditionHook
	{
		///SkyrimSE.exe+4442B0+87 is actually where we'd want to set up the formulas.

		
		static void Patch()
		{
			//should do this to the float version as well.
			auto script = RE::SCRIPT_FUNCTION::LocateScriptCommand("GetGraphVariableInt");

			func = script->conditionFunction;

			script->conditionFunction = thunk;

			
			logger::info("ConD1Hook complete...");
			//*/
		}

		//A seperate hook will have to be established to turn off the condition function check.

		static bool thunk(RE::TESObjectREFR* a_this, void* a2, void* a3, double& a4)
		{
			auto& arg = *reinterpret_cast<RE::BSFixedString*>(a2);

			
			if (auto str = arg.c_str(); !strnicmp(str, init_chars.data(), init_size) && arg.size() > init_size && std::isalnum(str[init_size]) == false)
			{
				//for now, I'm skipping everything until I get to a : character.
				
				size_t length = arg.size() - init_size;
				
				auto begin = str + init_size;

				//This needs to be a unique character, this cannot suffice. 
				// some unique string of characters that's immistakable, or use the parser. That's also viable.
				while (*begin != '\0' && strnicmp(begin, ">>", 2) != 0) length--, begin++;


				if (*begin != '\0')
				{
					length--, begin++;
					length--, begin++;



					std::string_view formula{ begin, length };

					logger::info("running '{}'", formula);

					a4 = Formula<double>::Run(formula, NAN);

					return true;
				}
				else
				{
					logger::info("error or something");
				}
			}
			else
				logger::info("launched '{}'", arg);

			return func(a_this, a2, a3, a4);
		}

		static inline RE::SCRIPT_FUNCTION::Condition_t* func = nullptr;
	};


	struct ConsoleHook
	{
		static void Patch()
		{
			//SE: 8DAE20, AE: 91CC70, VR: ???
			REL::RelocationID hook { 52065, 52952, 52065 };
			ptrdiff_t offset = REL::VariantOffset(0xE2, 0x52, 0xE2).offset();

			SKSE::AllocTrampoline(14);

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(hook.address() + offset, thunk);


			logger::info("ConD2Hook complete...");
			//*/
		}

		//A seperate hook will have to be established to turn off the condition function check.

		static bool thunk(RE::Script* a_this, RE::ScriptCompiler* a2, RE::COMPILER_NAME a3, RE::TESObjectREFR* a4)
		{
			logger::info("script check {}", a_this->text);

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

					logger::info("running '{}'", formula);

					try
					{
						Variable result = Formula<Variable>::Run(formula);

						auto log = RE::ConsoleLog::GetSingleton();

						if (result.IsVoid() == false)
							log->Print("result >> %s", result.PrintString().c_str());

						return true;
					}
					catch (const LEX::Error& error)
					{
						auto log = RE::ConsoleLog::GetSingleton();

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


	void Install()
	{

		//SE: 328110 + 75F//This can be used to get the result of a given console command call. I can use this to get the last value of the console
		// then load it into a thread local system or something like that, allowing it to return a value through a reference.

		ConditionHook::Patch();
		ConsoleHook::Patch();
	}

}