#pragma once

#include "Lexicon/Variable.h"

namespace LEX
{
	struct ConditionTLS
	{
		RE::ConditionCheckParams* params = nullptr;
		std::string_view filename;
		bool allowFilenameClear = true;
		Variable argument{};
		double prevSolution = NAN;
		bool preserveSolution = true;
		//double prevReturn = NAN;

		bool ShouldStoreSolution()
		{
			return this ? std::exchange(preserveSolution, true) : false;
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