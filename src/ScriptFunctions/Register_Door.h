#pragma once

namespace LEX::REG::DOOR
{
	//TODO: have a cached result class
	//Takes a pointer to itself, a pointer to it's parameter, and uses this to produce a cached result.


	inline int GetDoorFlags(RE::TESObjectDOOR* a_this)
	{
		if (!a_this)
			return 0;

		return a_this->flags.underlying();
	}



	inline void Register()
	{

		RegisterDump dump{ "Door" };
		dump = ProcedureHandler::instance->RegisterFunction(GetDoorFlags, "Shared::Door::GetDoorFlags");
	}
}