#pragma once



namespace LEX
{

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

			return static_cast<TypeOffset>(form ? form->GetFormType() : RE::FormType::None);
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

			return dynamic_cast<Form*>(form);
		}
	};
	
	template <std::derived_from<RE::TESForm> Form>
	struct VariableType<Form*>
	{
		AbstractTypePolicy* operator()(const Form* form)
		{
			auto offset = static_cast<TypeOffset>(form ? form->GetFormType() : Form::FORMTYPE);

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
	INITIALIZE()
	{

		//This gives 1 too many.
		RegisterObjectType<RE::TESForm*>("FORM", (TypeOffset)RE::FormType::Max);
		/*
		RE::TESForm* test = nullptr;
		
		//This may cause issues because it removes pointer?
		MakeObject(test);
		ObjectData to{};
		reinterpret_cast<RE::TESForm*&>(to) = test;
		FillObjectData<RE::TESForm*>(to, &test);
		ObjectTranslator<RE::TESForm*>{}(test);
		//*/
	}
}
