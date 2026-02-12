#pragma once

#include "Lexicon/Variable.h"

namespace LEX
{
	struct ConditionTLS
	{
		RE::ConditionCheckParams* params = nullptr;
		std::string_view filename;
		Variable argument{};

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


		void SetFilename(std::string_view file)
		{
			if (this)
			{
				filename = file;
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