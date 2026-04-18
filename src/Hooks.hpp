#pragma once

#include "Hooks/Hook_RunConsole.h"
#include "Hooks/Hook_FilesLoaded.h"
#include "Hooks/Hook_GetNoRumors.h"
#include "Hooks/Hook_LoadCondition.h"
#include "Hooks/Hook_ResolveCondition.h"
#include "Hooks/Hook_SaveConsoleResult.h"
#include "Hooks/Hook_SaveConditionParams.h"
#include "Hooks/Hook_SaveConditionResult.h"
namespace LEX
{
	inline void Install()
	{

		SKSE::AllocTrampoline(14 * 8);

		RunConsoleHook::Install();
		GetNoRumorsHook::Install();
		FilesLoadedHook::Install();
		LoadConditionHook::Install();
		ResolveConditionHook::Install();
		SaveConsoleResultHook::Install();
		SaveConditionParamsHook::Install();
		SaveConditionResultHook::Install();
	}

}