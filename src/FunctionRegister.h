#pragma once

namespace LEX
{
	RE::ActorValue LookupActorValueByName(const char* name)
	{
		//SE: 0x3E1450, AE: 0x3FC5A0
		using func_t = decltype(LookupActorValueByName);
		REL::Relocation<func_t> func{ RELOCATION_ID(26570, 27203) };
		return func(name);
	}



#define LEGACY_FUNCTION
#ifdef LEGACY_FUNCTION



	float GetActorValue_backend2(RE::Actor* a_this, LEX::String av_name, int flags)
	{
		constexpr int kBase = 1;
		constexpr int kPermanent = 2;
		constexpr int kTemporary = 4;
		constexpr int kDamage = 8;

		RE::ActorValue av = LookupActorValueByName(av_name);

		logger::debug("testing {}, av gotten {}", av_name.view(), magic_enum::enum_name(av));

		if (!a_this || av == RE::ActorValue::kNone)
			return 0;//Actually throw an exception.

		float value = 0;

		if (flags & kBase)
			value += a_this->AsActorValueOwner()->GetBaseActorValue(av);

		if (flags & kPermanent)
			value += a_this->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, av);

		if (flags & kTemporary)
			value += a_this->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
		
		if (flags & kDamage)
			value += a_this->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, av);

		if (isnan(value) == true)
			return 0;//Actually throw an exception.

		return value;
	}

	void ModActorValue(RE::Actor* a_this, String av_name, int a_modifier, float value)
	{

		RE::ActorValue av = LookupActorValueByName(av_name);

		if (!a_this || av == RE::ActorValue::kNone)
			return;//Actually throw an exception.

		RE::ACTOR_VALUE_MODIFIER modifier = (RE::ACTOR_VALUE_MODIFIER)a_modifier;

		switch (modifier)
		{
		case RE::ACTOR_VALUE_MODIFIER::kTotal:
			return a_this->AsActorValueOwner()->ModActorValue(av, value);

		case RE::ACTOR_VALUE_MODIFIER::kDamage:
		case RE::ACTOR_VALUE_MODIFIER::kPermanent:
		case RE::ACTOR_VALUE_MODIFIER::kTemporary:
			return a_this->AsActorValueOwner()->RestoreActorValue(modifier, av, value);

		default:
			//Invalid value used.
			break;
		}
	}

	void SetActorValue(RE::Actor* a_this, String av_name, float value)
	{

		RE::ActorValue av = LookupActorValueByName(av_name);

		if (!a_this || av == RE::ActorValue::kNone)
			return;//Actually throw an exception.



		return a_this->AsActorValueOwner()->SetBaseActorValue(av, value);
	}



	int GetLevel(RE::Actor* a_this)
	{
		if (!a_this) {
			//Report
			return 0;
		}

		return a_this->GetLevel();
	};


	bool IsInFaction(RE::Actor* a_this, RE::TESFaction* faction)
	{
		if (!a_this || !faction)
			return false;

		return a_this->IsInFaction(faction);
	};


	//These 2 should account for context I think.
	bool IsInFactionRank(RE::Actor* a_this, RE::TESFaction* faction, int min_rank, int max_rank)
	{

		if (!a_this || !faction)
			return false;

		bool inverted = min_rank > max_rank;

		if (inverted) {
			std::swap(min_rank, max_rank);
		}

		

		auto* faction_changes = a_this->extraList.GetByType<RE::ExtraFactionChanges>();

		auto fact_lamba = [=](auto& fact_data) {
			return fact_data.faction == faction &&
				fact_data.rank != -1 &&
				(fact_data.rank > min_rank - 1) == !inverted &&
				(fact_data.rank < max_rank + 1) == !inverted;
			};

		if (faction_changes)
		{
			auto end = faction_changes->factionChanges.end();
			auto it = std::find_if(faction_changes->factionChanges.begin(), end, fact_lamba);

			if (it != end)
				return true;
		}

		auto baseNPC = a_this->GetActorBase();
		
		if (baseNPC)
		{
			auto end = baseNPC->factions.end();
			auto it = std::find_if(baseNPC->factions.begin(), end, fact_lamba);

			if (it != end)
				return true;
		}
	
		//It shouldn't reach this point but it's whatever right now.

		return false;
	};



	int GetPerkCount(RE::Actor* a_this, LEX::String av_name)
	{
		//I seek to rework this.
		RE::ActorValue av = LookupActorValueByName(av_name);


		RE::ActorValueList* singleton = RE::ActorValueList::GetSingleton();

		if (!singleton) {
			//report
			logger::error("Actor Value List singleton has not loaded yet.");
			return 0;
		}

		if (!a_this)
			return 0;



		if (av == RE::ActorValue::kNone) {
			//report
			logger::error("Actor Value '{}' couldn't be found", av_name);
			return 0;
		}


		float count = 0;

		if (av < RE::ActorValue::kTotal)
		{
			RE::ActorValueInfo* info = singleton->GetActorValue(av);

			if (!info) {
				logger::error("Actor Value Info '{}' couldn't be found", av_name);
				return 0;
			}

			if (!info->perkTree) {
				logger::error("Actor Value '{}' has no perks.", av_name);
				return 0;
			}

			//No way to get the exact amount of what we are gonna get so I'm just gonna make it twenty.
			std::vector<RE::BGSSkillPerkTreeNode*> visited_nodes{};

			std::function<int(RE::BGSSkillPerkTreeNode*)> node_check = [&](RE::BGSSkillPerkTreeNode* node, int direction = 0) -> int
				{
					int count = 0;
					int add = 0;

					add = a_this->HasPerk(node->perk);

					count += add;

					if (add)
						return count;

					//It would seem the first entry is empty. Makes sense to tell if youve reached the first I guess?
					RE::BGSPerk* next_perk = node->perk ? node->perk->nextPerk : nullptr;

					while (next_perk)
					{
						add = a_this->HasPerk(next_perk);

						count += add;

						if (add)
							return count;


						next_perk = next_perk->nextPerk;
					}

					for (auto& child_node : node->children)
					{
						if (std::find(visited_nodes.begin(), visited_nodes.end(), child_node) != visited_nodes.end())
							continue;

						add = node_check(child_node);

						visited_nodes.push_back(child_node);

						if (add)
						{
							count += add;

							return count;
						}
					}

					return count;
				};

			return node_check(info->perkTree);
			//RE::BGSSkillPerkTreeNode* 
		}
		else
		{
			//Skills implementation, holding off on.
		}

		return 0;
	};


	int GetItemCount(RE::TESObjectREFR* a_this, RE::TESForm* form)
	{
		

		//Notice, I wish to improve this shit plz.
		

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



	float GetValue(RE::TESGlobal* a_this)
	{
		return a_this ? a_this->value : 0;
	};



	int GetMagicEffectCount(RE::Actor* a_this, RE::EffectSetting* effect, int is_active)
	{
		if (!a_this) {
			return 0;
		}

		if (!effect) {
			logger::error("effect is not an effect.");
			return 0;
		}


		float count = 0;


		RE::BSSimpleList<RE::ActiveEffect*>* effect_list = a_this->AsMagicTarget()->GetActiveEffectList();

		if (!effect_list || effect_list->empty() == true) {
			logger::error("{} doesn't have any active effects.", a_this->GetName());
			return 0;
		}

		for (auto active_effect : *effect_list)
		{
			bool active = active_effect->conditionStatus == RE::ActiveEffect::ConditionStatus::kTrue;

			//All other values are any.
			if (active && is_active != 1 || !active && is_active != 0)
				continue;

			if (active_effect->GetBaseObject() == effect) {
				count++;
			}

		}

		return count;
	};


	void GetGameSettingImpl(const char* setting_name, RE::Setting::Type type, void* out)
	{
		auto* singleton = RE::GameSettingCollection::GetSingleton();

		if (!singleton) {
			//report
			return;
		}
		RE::Setting* setting = singleton->GetSetting(setting_name);

		if (!setting) {
			//report
			return;
		}

		get_switch (setting->GetType())
		{
		case RE::Setting::Type::kBool:
			if (switch_value == type) {
				*static_cast<bool*>(out) = setting->GetBool();
			}

			break;

		case RE::Setting::Type::kFloat:
			if (switch_value == type) {
				*static_cast<float*>(out) = setting->GetFloat();
			}

			break;

		case RE::Setting::Type::kString:
			if (switch_value == type) {
				*static_cast<LEX::String*>(out) = setting->GetString();
			}

			break;



		case RE::Setting::Type::kSignedInteger:
			if (switch_value == type) {
				*static_cast<int32_t*>(out) = setting->GetSInt();
			}

			break;

		default:
			//report.
			logger::error("Setting type invalid.");
		};
	};


	template <typename T, RE::Setting::Type Type>
	T GetGameSetting(StaticTargetTag, LEX::String setting_name)
	{
		T out{};

		GetGameSettingImpl(setting_name.c_str(), Type, &out);

		return out;
	}



	bool IsRace(RE::Actor* a_this, RE::TESRace* race, bool use_original)
	{
		if (!a_this)
			return 0;

		//we have 
		//actor->GetActorBase()->originalRace
		//and if player, charGenRace
		// For now I'm going to use original race. Unsure if it'll work.

		RE::TESNPC* base = a_this->GetActorBase();

		RE::TESRace* act_race = use_original && base ? base->originalRace : a_this->GetActorRuntimeData().race;

		return act_race == race;

	};

	unsigned int GetFormID(RE::TESForm* a_this)
	{
		return a_this ? a_this->GetFormID() : 0;
	}

	unsigned int GetLocalFormID(RE::TESForm* a_this)
	{
		return a_this ? a_this->GetFormID() : 0;
	}


	double GetRandomRange(LEX::StaticTargetTag, double min, double max, uint32_t seed)
	{
		std::srand(seed == (uint32_t)-1 ? std::time(nullptr) : seed);
		
		//auto str = args[2]->GetStringParam();

		//logger::info("SentString is  \"{}\", min:{}, max:{}", str, min, max);

		//Flawed implementation, do not care.

		//Currently, this is basically an integer playing pretend as a double. Want to redo, don't care rn.

		int range = max - min + 1;

		int num = rand() % range + min;

		//cout << "Special Function: min = " << min << ", max = " << max << ", result is num " << num << ";";
		//ARTHMETIC_LOGGER(debug, "Special Function: min = {}, max = {}, result is num {};", min, max, num);
		return num;
	};

	bool HasKeyword(RE::TESForm* a_this, RE::BGSKeyword* keyword)
	{
		RE::BGSKeywordForm* key_form = a_this ? a_this->As<RE::BGSKeywordForm>() : nullptr;

		bool result = key_form ? key_form->HasKeyword(keyword) : false;

		return result;
	}
	
	bool HasKeywordString(RE::TESForm* a_this, LEX::String keyword)
	{
		RE::BGSKeywordForm* key_form = a_this ? a_this->As<RE::BGSKeywordForm>() : nullptr;

		bool result =  key_form ? key_form->HasKeywordString(keyword.view()) : false;

		return result;
	}

	bool HasKeywordInList(RE::TESForm* a_this, RE::BGSListForm* list, bool match_all)
	{
		return a_this ? a_this->HasKeywordInList(list, match_all) : false;
	}


	float GetCrimeGold(RE::Actor* a_this, int crime_type)
	{
		//Now in this sort of situation, I think it's pro

		RE::TESFaction* faction = nullptr;

		float result = 0;


		if (a_this->IsPlayerRef() == true) {
			RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

			for (auto& [faction, crime_struct] : player->GetCrimeValue().crimeGoldMap)
			{
				if (crime_type != 2)//IE if its ab
					result += crime_struct.nonViolentCur;

				if (crime_type != 1)//IE if its 2
					result += crime_struct.violentCur;
			}

			return result;
		}
		else
			faction = a_this->GetCrimeFaction();

		if (!faction)
			return 0;



		//if any other value than 1 or 2, gets both.
		if (crime_type != 2)//IE if its 1
			result += faction->GetCrimeGoldNonViolent();

		if (crime_type != 1)//IE if its 2
			result += faction->GetCrimeGoldViolent();

		return result;
	};


	unsigned int GetPlayerLevelDistance(RE::Actor* a_this, bool abs_value)
	{
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		if (!player || !a_this || a_this->IsPlayerRef())
			return 0;


		int value = player->GetLevel() - a_this->GetLevel();

		return abs_value ? abs(value) : value;
	};


	RE::TESForm* LookupByFormID(StaticTargetTag, RE::FormID id)
	{
		auto form = RE::TESForm::LookupByID(id);
		logger::info("ID {:X} {}", id, !!form);
		return form;
	};

	RE::TESForm* LookupByLocalID(StaticTargetTag, String plugin, RE::FormID id)
	{
		RE::TESForm* form = nullptr;

		if (auto data_handler = RE::TESDataHandler::GetSingleton(); data_handler) {
			form = data_handler->LookupForm(id, plugin.view());
		}

		return form;
	};

	RE::TESForm* LookupByEditorID(StaticTargetTag, String editor_id)
	{
		auto result = RE::TESForm::LookupByEditorID(editor_id.view());

		return result;
	};

	void TESTTHING()
	{
		//using Type = int;
		//int Type = 2;
		//auto test = (Type)!1.f;
	}




	struct RegisterDump
	{
		inline static int no = 0;

		RegisterDump& operator=(bool result)
		{
			logger::info("#{} = {}", ++no, result);
			return *this;
		}
	};

	void temp_NativeFormulaRegister()
	{	
		RegisterDump dump;
			
			
			
		dump = ProcedureHandler::instance->RegisterFunction(GetActorValue_backend2, "Shared::GameObjects::GetActorValue2");				//01
		dump = ProcedureHandler::instance->RegisterFunction(GetLevel, "Shared::GameObjects::GetLevel");									//02
		dump = ProcedureHandler::instance->RegisterFunction(IsInFaction, "Shared::GameObjects::IsInFaction");							//03
		dump = ProcedureHandler::instance->RegisterFunction(IsInFactionRank, "Shared::GameObjects::IsInFactionRank");					//04
		dump = ProcedureHandler::instance->RegisterFunction(GetPerkCount, "Shared::GameObjects::GetPerkCount");							//05
		dump = ProcedureHandler::instance->RegisterFunction(GetItemCount, "Shared::GameObjects::GetItemCount");							//06
		dump = ProcedureHandler::instance->RegisterFunction(GetValue, "Shared::GameObjects::GetValue");									//07
		dump = ProcedureHandler::instance->RegisterFunction(GetMagicEffectCount, "Shared::GameObjects::GetMagicEffectCount");			//08
		dump = ProcedureHandler::instance->RegisterFunction(IsRace, "Shared::GameObjects::IsRace");										//09
		dump = ProcedureHandler::instance->RegisterFunction(GetFormID, "Shared::GameObjects::GetFormID");								//10
		dump = ProcedureHandler::instance->RegisterFunction(GetLocalFormID, "Shared::GameObjects::GetLocalFormID");						//11
		dump = ProcedureHandler::instance->RegisterFunction(GetRandomRange, "Shared::GameObjects::GetRandomRange");						//12
		dump = ProcedureHandler::instance->RegisterFunction(HasKeyword, "Shared::GameObjects::HasKeyword");								//13
		dump = ProcedureHandler::instance->RegisterFunction(HasKeywordString, "Shared::GameObjects::HasKeywordString");					//14
		dump = ProcedureHandler::instance->RegisterFunction(HasKeywordInList, "Shared::GameObjects::HasKeywordInList");					//15
		dump = ProcedureHandler::instance->RegisterFunction(GetCrimeGold, "Shared::GameObjects::GetCrimeGold");							//16
		dump = ProcedureHandler::instance->RegisterFunction(GetPlayerLevelDistance, "Shared::GameObjects::GetPlayerLevelDistance");		//17
		
		dump = ProcedureHandler::instance->RegisterFunction(GetGameSetting<float, RE::Setting::Type::kFloat>, "Shared::GameObjects::GetGameSettingFloat");		//18
		dump = ProcedureHandler::instance->RegisterFunction(GetGameSetting<bool, RE::Setting::Type::kBool>, "Shared::GameObjects::GetGameSettingBool");			//19
		dump = ProcedureHandler::instance->RegisterFunction(GetGameSetting<String, RE::Setting::Type::kString>, "Shared::GameObjects::GetGameSettingString");	//20
		dump = ProcedureHandler::instance->RegisterFunction(GetGameSetting<int, RE::Setting::Type::kSignedInteger>, "Shared::GameObjects::GetGameSettingInt");	//21
		


		
		dump = ProcedureHandler::instance->RegisterFunction(LookupByFormID, "Shared::GameObjects::LookupByFormID");			//22
		dump = ProcedureHandler::instance->RegisterFunction(LookupByLocalID, "Shared::GameObjects::LookupByLocalID");		//23
		dump = ProcedureHandler::instance->RegisterFunction(LookupByEditorID, "Shared::GameObjects::LookupByEditorID");		//24


		dump = ProcedureHandler::instance->RegisterFunction(SetActorValue, "Shared::GameObjects::SetActorValue");			//25
		dump = ProcedureHandler::instance->RegisterFunction(ModActorValue, "Shared::GameObjects::ModActorValue");			//26
	}

#endif
}