#pragma once


#include "FunctorManager.h"

#include "ConditionTLS.h"

namespace LEX
{

	struct GetNoRumorsHook
	{
		///SkyrimSE.exe+4442B0+87 is actually where we'd want to set up the formulas.


		static void Install()
		{
			auto script = RE::SCRIPT_FUNCTION::LocateScriptCommand("GetNoRumors");

			func = script->conditionFunction;

			script->conditionFunction = thunk;
		}

		//A seperate hook will have to be established to turn off the condition function check.

		static bool thunk(RE::TESObjectREFR* a_this, void* param1, void* param2, double& result)
		{
			bool should_set = true;

			bool ret = true;

			RE::TESObjectREFR* subject;
			RE::TESObjectREFR* target;

			if (auto params = currentParams->GetParams()) {
				subject = params->actionRef;
				target = params->targetRef;
			}
			else {
				subject = target = nullptr;
			}

			float solution = currentParams->GetSolution();
			
			switch (reinterpret_cast<size_t>(param2))
			{
			case 0xDEADBEEF:
				if  constexpr (1)
				{
					if (!param1) {
						result = NAN;
						break;
					}
					auto& formula = *reinterpret_cast<ConditionFormula*>(param1);

					result = formula ? formula(a_this)->Call(Property::PopArgument(), subject, target, solution, 0) : 0;
				}
				break;

			case FunctorManager::k_argFuncCode:
				currentParams->solveToArg = true;
				[[fallthrough]];

			case FunctorManager::k_exFuncCode:
				if  constexpr (1)
				{
					Functor* functor = reinterpret_cast<Functor*>(param1);
					result = functor->Execute(a_this, subject, target, solution);
					should_set = functor ? functor->isSolvable : true;
				}
				break;

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
				ret = func(a_this, param1, param2, result);
				break;
			}

			if (currentParams)
				currentParams->preserveSolution = should_set;

			return ret;

		}


		static inline RE::SCRIPT_FUNCTION::Condition_t* func = nullptr;
	};

}