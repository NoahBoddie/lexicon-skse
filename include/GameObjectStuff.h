#pragma once

#include "Lexicon/VariableType.h"
#include "Lexicon/ObjectInfo.h"
#include "Lexicon/Object.h"

#include "ClibUtil/string.hpp"
#include "ClibUtil/editorID.hpp"

#include "MergeMapperPluginAPI.h"




//TODO: Place make a file called LexiconSKSE.hpp that holds all the required headers. This doesn't import easy
namespace LEX
{
	//*
	ENUM(ExtraForm)
	{
		//This is a list of forms that other forms derive from, allowing for a far greater representation of what an object is.
		kPlayerCharacter,
		kMagicItem,
		kDescriptionForm,
		kFood,
		kPoison,
		kPotion,


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
				offset = GetExtraFormOffset(ExtraForm::kPlayerCharacter);
			}

			else if constexpr (std::is_same_v<RE::MagicItem, Form>)
			{
				offset = GetExtraFormOffset(ExtraForm::kMagicItem);
			}
			else
			{
				if (form && form->IsPlayerRef() == true) {
					offset = GetExtraFormOffset(ExtraForm::kPlayerCharacter);
				}
				else {
					offset = static_cast<TypeOffset>(form ? form->GetFormType() : Form::FORMTYPE);
				}
			}
			
			return IdentityManager::instance->GetTypeByOffset("FORM", offset)->FetchTypePolicy(nullptr);
		}
	};



	void Test()
	{
		VariableType<RE::TESForm*>{};
		constexpr bool value = detail::call_class_has_var_type<RE::TESForm*>;
		//GetVariableType<RE::TESForm*>();
	}



//Please move this.
#ifdef LEX_SOURCE

	//An extension of default client to be able to handle the extensions for linking with 
	struct DefaultClientEx : public DefaultClient
	{


		virtual TypeOffset HandleExtraOffsetArgs(std::string_view category, std::string_view* data, size_t length) override
		{ 

			switch (Hash(category))
			{
			case "FORM"_h:
			{
				if (length != 1)
					report::compile::critical("FORM requires 1 entry");

				//I can make a macro for most of this

#define RETURN_RE_TYPE_OFFSET(mc_name) case #mc_name##_h: return static_cast<TypeOffset>(RE::mc_name::FORMTYPE)
#define RETURN_EX_TYPE_OFFSET(mc_name) case #mc_name##_h: return GetExtraFormOffset(ExtraForm::k##mc_name)
				switch (Hash(data[0]))
				{
					RETURN_RE_TYPE_OFFSET(IngredientItem);
					RETURN_RE_TYPE_OFFSET(AlchemyItem);
					RETURN_RE_TYPE_OFFSET(SpellItem);
					RETURN_RE_TYPE_OFFSET(ScrollItem);
					RETURN_RE_TYPE_OFFSET(EnchantmentItem);
					RETURN_RE_TYPE_OFFSET(Character);
					RETURN_RE_TYPE_OFFSET(TESObjectREFR);
					RETURN_RE_TYPE_OFFSET(BGSKeyword);
					RETURN_RE_TYPE_OFFSET(TESGlobal);
					RETURN_RE_TYPE_OFFSET(TESRace);
					RETURN_RE_TYPE_OFFSET(BGSListForm);
					RETURN_RE_TYPE_OFFSET(TESFaction);
					RETURN_RE_TYPE_OFFSET(EffectSetting);

					RETURN_EX_TYPE_OFFSET(MagicItem);
					RETURN_EX_TYPE_OFFSET(PlayerCharacter);


					default:
						report::compile::critical("Unknown name '{}' used", data[0]);
				}

#undef RETURN_RE_TYPE_OFFSET;
#undef RETURN_EX_TYPE_OFFSET;
			}




			default: break;
			}
			return -1;
		}

		std::string_view GetCompileOptions(size_t index) override 
		{ 
			if (index == 0)
				return "Skyrim";
			
			return {}; 
		}

	};

#endif

	//*/
}
