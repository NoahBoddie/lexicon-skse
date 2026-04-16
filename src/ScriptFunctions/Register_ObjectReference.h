#pragma once

#include "Utility.h"

namespace LEX::REG::REFR
{
	//TODO: have a cached result class
	//Takes a pointer to itself, a pointer to it's parameter, and uses this to produce a cached result.


	inline RE::TESObjectCELL* GetParentCell(RE::TESObjectREFR* a_this)
	{
		if (!a_this) {
			//Report
			return {};
		}


		return a_this->GetParentCell();
	}

	inline RE::TESObjectREFR* GetTeleportReference(RE::TESObjectREFR* a_this)
	{
		if (!a_this) {
			//Report
			return {};
		}

		auto teleport = a_this->extraList.GetByType<RE::ExtraTeleport>();

		if (!teleport || !teleport->teleportData)
			return nullptr;

		return teleport->teleportData->linkedDoor.get().get();
	}

	inline bool IsObjective(RE::TESObjectREFR* a_this, RE::TESQuest* quest, bool active_only)
	{
		if (!a_this)
		{
			//Kinda need to report this
			return {};
		}

		//This kind of method seemingly only works when the target is an actor. I can imagine it's to handle
		// being essential and shit.

		auto aliases = a_this->extraList.GetByType<RE::ExtraAliasInstanceArray>();

		if (aliases)
		{
			std::set<std::pair<RE::TESQuest*, uint32_t>> search;

			for (RE::BGSRefAliasInstanceData* data : aliases->aliases)
			{
				if (quest && data->quest != quest)
					continue;

				search.emplace(data->quest, data->alias->aliasID);
			}

			if (search.empty() == false)
			{
				static RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

				for (RE::BGSInstancedQuestObjective& objective : player->GetPlayerRuntimeData().objectives)
				{
					if (active_only && objective.InstanceState != RE::QUEST_OBJECTIVE_STATE::kDisplayed)
						continue;

					auto owner_quest = objective.Objective->ownerQuest;

					if (quest && owner_quest != quest)
						continue;


					std::span targets{ objective.Objective->targets, objective.Objective->numTargets };

					for (auto target : targets)
					{
						//owner_quest->GetAliasedRef(target->alias);
						//target->alias;

						if (search.contains(std::make_pair(owner_quest, target->alias)) == true)
							return true;
					}


				}

			}
		}

		return false;
	}


	inline RE::TESBoundObject* GetBaseObject(RE::TESObjectREFR* a_this)
	{
		if (!a_this) {
			//Report
			return {};
		}

		return a_this->GetBaseObject();
	}

	inline int GetItemCount(RE::TESObjectREFR* a_this, RE::TESForm* form)
	{


		//TODO: Notice, I wish to improve this shit plz.


		if (!a_this) {
			//Report
			return 0;
		}
		//I will make this support formlists later.

		RE::TESBoundObject* object = form->As<RE::TESBoundObject>();

		if (!object) {
			return 0;
		}
		auto inventory_counts = a_this->GetInventoryCounts();

		return inventory_counts[object];
	};



	inline void Register()
	{

		RegisterDump dump{ "ObjectReference" };
		dump = ProcedureHandler::instance->RegisterFunction(GetItemCount, "Shared::ObjectReference::GetItemCount");					//00
		dump = ProcedureHandler::instance->RegisterFunction(GetBaseObject, "Shared::ObjectReference::GetBaseObject");				//01
		dump = ProcedureHandler::instance->RegisterFunction(IsObjective, "Shared::ObjectReference::IsObjective");					//02
		dump = ProcedureHandler::instance->RegisterFunction(GetTeleportReference, "Shared::ObjectReference::GetTeleportReference");	//03
		dump = ProcedureHandler::instance->RegisterFunction(GetParentCell, "Shared::ObjectReference::GetParentCell");				//04
	}
}