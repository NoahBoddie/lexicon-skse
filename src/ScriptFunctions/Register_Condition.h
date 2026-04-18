#pragma once

#include "ConditionTLS.h"

namespace LEX::REG::COND
{
	



	inline Voidable GetProperty(StaticTargetTag, std::string_view name)
	{
		return {};
	}

	inline double GetSolution(StaticTargetTag)
	{
		return currentParams ? currentParams->prevSolution : 0;
	}

	inline void Register()
	{

		RegisterDump dump{ "Condition" };
		dump = ProcedureHandler::instance->RegisterFunction(GetProperty, "Shared::Condition::GetProperty");
		dump = ProcedureHandler::instance->RegisterFunction(GetSolution, "Shared::Condition::GetSolution");
	}
}