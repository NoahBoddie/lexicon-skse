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
			
			ParameterCode code = param2;

			switch (code.code)
			{
			case 0xDEADBEEF:
				if  constexpr (1)
				{
					if (!param1) {
						result = NAN;
						currentParams->SetForceResult(false);
						break;
					}
					auto& formula = *reinterpret_cast<ConditionFormula*>(param1);
					//Might be able to vary the args off of something like a parameter
					result = formula ? formula(a_this)->Call(subject, target, solution, 0) : 0;
					currentParams->SetPreserves(true);
					if (!formula)
						currentParams->SetForceResult(false);
				}
				break;

			case FunctorManager::k_argFuncCode.code:
				if  constexpr (1)
				{
					Functor* functor = reinterpret_cast<Functor*>(param1);
					result = functor->Execute(a_this, subject, target, solution);
					
					bool success = functor && functor->IsSolvable();

					if (success = code.data > 0)
						currentParams->LoadArgument(code.data--, result);

					currentParams->SetForceResult(success);
					should_set = false;
				}
				break;

			case FunctorManager::k_exFuncCode.code:
				if  constexpr (1)
				{
					Functor* functor = reinterpret_cast<Functor*>(param1);
					result = functor->Execute(a_this, subject, target, solution);
					should_set = functor && functor->IsSolvable();
					if (!functor)
						currentParams->SetForceResult(false);
					else if (functor->IsSolvable() == false)
						currentParams->SetForceResult(true);
				}
				break;

			case FunctorManager::k_loadPropCode.code:
				if  constexpr (1)
				{
					Property* property = reinterpret_cast<Property*>(param1);
					currentParams->SetForceResult(property->Load(code.data));
					result = 1;
				}
				return true;

			case FunctorManager::k_loadFileCode.code:
				currentParams->SetForceResult(!!param1);
				should_set = false;
				result = 0;
				return true;

			default:
				ret = func(a_this, param1, param2, result);
				break;
			}

			currentParams->SetPreserves(should_set);

			return ret;

		}


		static inline RE::SCRIPT_FUNCTION::Condition_t* func = nullptr;
	};

}