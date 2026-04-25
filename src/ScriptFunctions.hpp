#pragma once

#include "ScriptFunctions/Register_Door.h"
#include "ScriptFunctions/Register_Location.h"
#include "ScriptFunctions/Register_Condition.h"
#include "ScriptFunctions/Register_ObjectReference.h"


namespace LEX::REG
{

	INITIALIZE("function_register")
	{
		COND::Register();
		DOOR::Register();
		LCTN::Register();
		REFR::Register();
	}
}