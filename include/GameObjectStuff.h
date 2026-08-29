#pragma once

#include "Lexicon/VariableType.h"
#include "Lexicon/ObjectInfo.h"
#include "Lexicon/Object.h"

#include "ClibUtil/string.hpp"
#include "ClibUtil/editorID.hpp"



//TODO: Place make a file called LexiconSKSE.hpp that holds all the required headers. This doesn't import easy
namespace LEX
{
	//*
	ENUM(ExtraForm)
	{
		//This is a list of forms that other forms derive from, allowing for a far greater representation of what an object is.
		PlayerCharacter,
		MagicItem,
		DescriptionForm,
		Food,
		Poison,
		Potion,
		BoundObject,
		BoundAnimObject,

		kTotal,


		kMaxFormTypes = ExtraForm::kTotal + (TypeOffset)RE::FormType::Max,
	};


	inline TypeOffset GetExtraFormOffset(ExtraForm id)
	{
		return (TypeOffset)RE::FormType::Max + id;
	}


	template <std::derived_from<RE::TESForm> Form>
	struct ObjectTranslator<Form*>
	{
		RE::TESForm* operator()(const Form* form)
		{
			return unconst(form);

		}
	};

	template <>
	struct ObjectSettings<RE::TESForm*>
	{
		OBJECT_INFO_DATA(1)
		{
			"FORM", 
			ExtraForm::kMaxFormTypes
		};
	};







	template <std::derived_from<RE::TESForm> Form>
	struct Unvariable<Form*>
	{
		Form* operator()(Variable* var)
		{
			Object type = var->AsObject();

			RE::TESForm* form = type.get<RE::TESForm*>();
			
			//report::debug("form is named {} with formtype {}", form->GetName(), magic_enum::enum_name(form->GetFormType()));

			if constexpr (std::is_same_v<Form, RE::TESForm>)
				return form;
			else
				return skyrim_cast<Form*>(form);
		}
	};
	
	//The ability to get the dynamic type should come from one function, and that function that both the object and other share.
	// This function is to get the offset basically.

	template <std::derived_from<RE::TESForm> Form>
	struct VariableType<Form*>
	{
		TypeInfo* operator()(const Form* form)
		{

			TypeOffset offset;


			if constexpr (std::is_same_v<RE::PlayerCharacter, Form>)
			{
				offset = GetExtraFormOffset(ExtraForm::PlayerCharacter);
			}

			else if constexpr (std::is_same_v<RE::MagicItem, Form>)
			{
				offset = GetExtraFormOffset(ExtraForm::MagicItem);
			}
			else if constexpr (std::is_same_v<RE::TESBoundObject, Form>)
			{
				offset = GetExtraFormOffset(ExtraForm::BoundObject);
			}
			else if constexpr (std::is_same_v<RE::TESBoundAnimObject, Form>)
			{
				offset = GetExtraFormOffset(ExtraForm::BoundAnimObject);
			}

			else
			{
				if (form && form->IsPlayerRef() == true) {
					offset = GetExtraFormOffset(ExtraForm::PlayerCharacter);
				}
				else {
					offset = static_cast<TypeOffset>(form ? form->GetFormType() : Form::FORMTYPE);
				}
			}
			
			return NULL_OP(NULL_Q(IdentityManager::instance->GetTypeByOffset("FORM", offset))->GetTypeInfo(nullptr));
		}
	};


	template <>
	struct VariableType<RE::BSFixedString>
	{

		TypeInfo* operator()()
		{
			return String::GetVariableType(nullptr);
		}
	};
	using CRIME_TYPE = RE::PackageNS::CRIME_TYPE;

	//TODO: Implement the shared enums.
	
	SCRIPT_ENUM_NAME(RE::FormType, "Shared::Enums::FormType");
	SCRIPT_ENUM_NAME(RE::ActorValue, "Shared::Enums::ActorValue");
	SCRIPT_ENUM_NAME(RE::PackageNS::CRIME_TYPE, "Shared::Enums::CrimeType");
	void Test()
	{
		VariableType<RE::TESForm*>{};
		constexpr bool value = detail::call_class_has_var_type<RE::TESForm*>;
		//GetVariableType<RE::TESForm*>();
	}
}
