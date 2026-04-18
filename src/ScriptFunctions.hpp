#pragma once

#include "ScriptFunctions/Register_Door.h"
#include "ScriptFunctions/Register_Condition.h"
#include "ScriptFunctions/Register_ObjectReference.h"


namespace LEX::REG
{

	INITIALIZE("function_register")
	{
		DOOR::Register();
		COND::Register();
		REFR::Register();
	}
}