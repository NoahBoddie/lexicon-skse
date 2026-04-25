#pragma once

#include "Lexicon/Variable.h"

namespace LEX
{
	//Automate the existence of this some. Like, I can probably make it auto set the current or something and set the 
	// previous when it dies.
	struct ConditionTLS
	{
		RE::ConditionCheckParams* params = nullptr;
		std::string_view filename;
		bool allowFilenameClear = true;
		Variable argument{};
		float prevSolution = 0;
		bool preserveSolution = true;
		bool solveToArg = false;
		//double prevReturn = NAN;

		bool ShouldStoreSolution()
		{
			return this ? std::exchange(preserveSolution, true) : false;
		}

		bool ShouldSolveArgument()
		{
			return this ? std::exchange(solveToArg, false) : false;
		}

		float GetSolution()
		{
			return this ? prevSolution : 0;
		}


		RE::ConditionCheckParams* GetParams()
		{
			return this ? params : nullptr;
		}

		void ClearArgument()
		{
			if (this) {
				argument = Variable{};
			}
		}

		bool SetArgument(Variable arg)
		{
			if (this) {
				argument = std::move(arg);
				return true;
			}

			return false;
		}


		void SetFilename(std::string_view file, bool allow_clear = true)
		{
			if (this){
				filename = file;
				allowFilenameClear = allow_clear;
			}
		}

		void ClearFilename()
		{
			if (this && allowFilenameClear) {
				filename = {};
			}
		}

		void AddFilename(std::string& name)
		{
			if (this && filename.empty() == false)
			{
				name = std::format("{}@{}", name, filename);
			}
		}



		Variable GetArgument()
		{
			return this ? argument : Variable{};
		}
	};

	inline static thread_local ConditionTLS* currentParams = nullptr;

}