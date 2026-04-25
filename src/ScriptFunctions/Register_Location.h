#pragma once

namespace LEX::REG::LCTN
{
	inline RE::BGSLocation* GetParentLocation(RE::BGSLocation* a_this)
	{
		if (!a_this) {
			return nullptr;
		}

		return a_this->parentLoc;
	}




	inline void Register()
	{

		RegisterDump dump{ "Location" };
		dump = ProcedureHandler::instance->RegisterFunction(GetParentLocation, "Shared::GameObjects::GetParentLocation");
	}
}