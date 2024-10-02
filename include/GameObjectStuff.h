#pragma once



namespace LEX
{

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



	template <>
	struct LEX::ObjectInfo<RE::TESForm*> : public QualifiedObjectInfo<RE::TESForm*>
	{
		using Typer = int;

		bool IsPooled(ObjectData&) override
		{
			return false;
		}

		TypeOffset GetTypeOffset(ObjectData& data) override
		{
			auto form = data.get<RE::TESForm*>();

			if (form->IsPlayerRef() == true) {
				return GetExtraFormOffset(ExtraForm::kPlayerCharacter);
			}


			return static_cast<TypeOffset>(form ? form->GetFormType() : RE::FormType::None);
		}

		//the form object info needs to edit the transfer functions,


		void TryDetach(RE::TESForm*& target)
		{
			if (target && target->IsPlayerRef() == false)
			{
				if (auto target_ref = target->AsReference(); target_ref) {
					target_ref->DecRefCount();
				}

				target = nullptr;
			}
		}

		void TryAttach(RE::TESForm* target)
		{
			if (target && target->IsPlayerRef() == false)
			{
				if (auto target_ref = target->AsReference(); target_ref)
					target_ref->IncRefCount();
			}
		}

		void Copy(ObjectData& self, const ObjectData& other) override
		{
			
			RE::TESForm*& a_self = self.get<Type>();
			RE::TESForm*& a_other = other.get<Type>();

			if (a_self != a_other){
				TryDetach(a_self);	
			}
			
			__super::Copy(self, other);

			TryAttach(a_self);
		
		}


		void Destroy(ObjectData& self) override
		{
			
			TryDetach(self.get<Type>());
			
			__super::Destroy(self);
		}
	};


	//template <std::derived_from<RE::TESForm> Form>
	//decltype(auto) ToObject<Form*>(Form*& obj)
	//{
	//	return static_cast<RE::TESForm*>(obj);
	//}


	template <std::derived_from<RE::TESForm> Form>
	struct ObjectTranslator<Form*>
	{
		RE::TESForm* operator()(Form*& val)
		{
			return val;

		}
	};


	template <std::derived_from<RE::TESForm> Form>
	struct Unvariable<Form*>
	{
		Form* operator()(Variable* var)
		{
			Object type = var->AsObject();

			RE::TESForm* form = type.get<RE::TESForm*>();
			
			//report::debug("form is named {} with formtype {}", form->GetName(), magic_enum::enum_name(form->GetFormType()));


			return skyrim_cast<Form*>(form);
		}
	};
	
	//The ability to get the dynamic type should come from one function, and that function that both the object and other share.
	// This function is to get the offset basically.

	template <std::derived_from<RE::TESForm> Form>
	struct VariableType<Form*>
	{
		AbstractTypePolicy* operator()(const Form* form)
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



	/*
	template <pointer_derived_from<RE::TESForm> FormPtr>
	struct VariableType<FormPtr>
	{
		using Form = std::remove_pointer_t<FormPtr>;

		AbstractTypePolicy* operator()(const FormPtr* form)
		{
			return VariableType<Form>{}(form ? *form : nullptr);
		}
	};
	//*/

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

		std::string_view GetCompileOptions(size_t index) const override 
		{ 
			if (index == 0)
				return "Skyrim";
			
			return {}; 
		}

	};

#endif
}
