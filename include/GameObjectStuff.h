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

		kTotal
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
	struct LEX::ObjectInfo<RE::TESForm*> : public INTERNAL_OBJECT_INFO(RE::TESForm*)
	{
#ifdef LEX_SOURCE

		bool IsPooled(ObjectData&) override;


		virtual void Initialize(ObjectData& data) override;


		TypeOffset GetTypeOffset(ObjectData& data) override;

		//the form object info needs to edit the transfer functions,


		void TryDetach(RE::TESForm*& target);

		void TryAttach(RE::TESForm* target);

		void Copy(ObjectData& self, const ObjectData& other) override;


		void Destroy(ObjectData& self) override;

		String PrintString(ObjectData& a_self, std::string_view context) override;

		bool CreateLiteralData(std::string_view literal, uintptr_t & hash, ObjLitCtor & ctor) override;

		TypeOffset GetOffsetFromArgs(const std::string_view& category, const std::span<std::string_view>& args) override;
#endif
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



	void Test()
	{
		VariableType<RE::TESForm*>{};
		constexpr bool value = detail::call_class_has_var_type<RE::TESForm*>;
		//GetVariableType<RE::TESForm*>();
	}
}
