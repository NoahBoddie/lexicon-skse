#pragma once

#include "Lexicon/Variable.h"
#include "Parameter.h"
namespace LEX
{
	enum struct ForceResult
	{
		NA,
		True,
		False,
	};

	//Automate the existence of this some. Like, I can probably make it auto set the current or something and set the 
	// previous when it dies.
	struct ConditionTLS
	{
		//inline static thread_local ConditionTLS* current = nullptr;
		//ConditionTLS* previousTLS = nullptr;


		RE::ConditionCheckParams* params = nullptr;
		std::string_view filename;
		float prevSolution = 0;
		ForceResult force = ForceResult::NA;
		std::vector<Variable> arguments;

		bool allowFilenameClear = true;
		bool preserveSolution = true;


		bool LoadArgument(uint32_t index, Variable value)
		{
			if (this)
			{
				assert_if (index > Parameter::MAX_SIZE) {
					//Outside of the range, this shouldn't happen
					return false;
				}

				if (arguments.size() <= index)
					arguments.resize(index + 1);

				arguments[index] = std::move(value);
			}

			return this;
		}

		inline bool PopArguments(uint32_t min, uint32_t size, std::vector<Variable>& args)
		{
			//If the number of args are less than the size of parameters, but more than the minimum, it won't pad the arg 
			// collection. If the number of args is below the minimum it will fail to execute.

			if (min == -1) {
				min = size;
			}

			if (arguments.size() > min)
				return false;

			std::vector<Variable> result;

			auto range = std::min<size_t>(arguments.size(), min);

			arguments.resize(range);

			args = std::move(arguments);

			return true;
		}

		void SetPreserves(bool value)
		{
			if (this)
				preserveSolution = value;
		}

		void SetForceResult(bool value)
		{
			if (this)
			force = value ? ForceResult::True : ForceResult::False;
		}

		void CheckResult(bool& result)
		{
			if (this)
			{
				switch (force)
				{
				case ForceResult::True:
					result = true;
					break;
				case ForceResult::False:
					result = false;
					break;
				}

				force = ForceResult::NA;
			}
		}

		//double prevReturn = NAN;

		bool ShouldStoreSolution()
		{
			return this ? std::exchange(preserveSolution, true) : false;
		}

		void TrySolution(float value)
		{
			if (this)
			{
				if (std::exchange(preserveSolution, true) == true)
				{
					prevSolution = value;
				}
			}
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
				arguments.clear();
			}
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


	};

	inline static thread_local ConditionTLS* currentParams = nullptr;

}