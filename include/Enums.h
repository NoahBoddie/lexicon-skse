#pragma once

namespace LEX
{
	ENUM(AVModiferFlag, uint8_t)
	{
		None = 0,
		Base = 1 << 0,
		Permanent = 1 << 1,
		Temporary = 1 << 2,
		Damage = 1 << 3,
		All = Base | Permanent | Temporary | Damage,
		//Desirable combinations
		Natural = Base | Permanent,
		Increase = Permanent | Temporary,
		Maximum = Base | Permanent | Temporary,
		Cached = Natural,             //Those values that are persistent
		Loaded = Temporary | Damage,  //Values that are only for loaded actors, barring cases withstanding.

		//Exception combinations
		ExceptBase = All ^ Base,
		ExceptPermanent = All ^ Permanent,
		ExceptTemporary = All ^ Temporary,
		ExceptDamage = All ^ Damage,

	};



	SCRIPT_ENUM_NAME(RE::FormType, "Shared::Enums::FormType");
	SCRIPT_ENUM_NAME(RE::ActorValue, "Shared::Enums::ActorValue");
	SCRIPT_ENUM_NAME(AVModiferFlag, "Shared::Enums::AVModiferFlag");
	SCRIPT_ENUM_NAME(RE::PackageNS::CRIME_TYPE, "Shared::Enums::CrimeType");
	SCRIPT_ENUM_NAME(RE::ACTOR_VALUE_MODIFIER, "Shared::Enums::ActorValueModifier");
}