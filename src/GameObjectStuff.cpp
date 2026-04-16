#include "GameObjectStuff.h"

#include "MergeMapperPluginAPI.h"
#include "ScriptFunctions.hpp"

namespace LEX
{
#ifdef LEX_SOURCE
	bool LEX::ObjectInfo<RE::TESForm*>::IsPooled(ObjectData&)
	{
		return false;
	}


	void LEX::ObjectInfo<RE::TESForm*>::Initialize(ObjectData& data)
	{
		auto form = data.get<RE::TESForm*>();
		TryAttach(form);
	}


	TypeOffset LEX::ObjectInfo<RE::TESForm*>::GetTypeOffset(ObjectData& data)
	{
		auto form = data.get<RE::TESForm*>();

		if (!form) {
			return static_cast<TypeOffset>(RE::FormType::None);
		}

		if (form->IsPlayerRef() == true) {
			return GetExtraFormOffset(ExtraForm::PlayerCharacter);
		}


		return static_cast<TypeOffset>(form->GetFormType());
	}

	//the form object info needs to edit the transfer functions,


	void LEX::ObjectInfo<RE::TESForm*>::TryDetach(RE::TESForm*& target)
	{
		if (target)
		{
			if (auto target_ref = target->As<RE::TESObjectREFR>(); target_ref) {
				auto before = target_ref->BSHandleRefObject::QRefCount();
				target_ref->DecRefCount();
				report::message::trace("Decrement {} ref count: {} -> {}", target_ref->GetDisplayFullName(), before, target_ref->BSHandleRefObject::QRefCount());
			}

			target = nullptr;
		}
	}


	RE::NiPointer<RE::TESObjectREFR> pointer = nullptr;

	void LEX::ObjectInfo<RE::TESForm*>::TryAttach(RE::TESForm* target)
	{
		if (target)
		{
			if (auto target_ref = target->As<RE::TESObjectREFR>(); target_ref) {
				pointer.reset(target_ref);
				auto before = target_ref->BSHandleRefObject::QRefCount();
				target_ref->IncRefCount();
				report::message::trace("Increment {} ref count: {} -> {}", target_ref->GetDisplayFullName(), before, target_ref->BSHandleRefObject::QRefCount());

			}
		}
	}

	void LEX::ObjectInfo<RE::TESForm*>::Copy(ObjectData& self, const ObjectData& other)
	{

		RE::TESForm*& a_self = self.get<Type>();
		RE::TESForm*& a_other = other.get<Type>();

		if (a_self != a_other) {
			TryDetach(a_self);
			__super::Copy(self, other);
			TryAttach(a_self);

		}

	}


	void LEX::ObjectInfo<RE::TESForm*>::Destroy(ObjectData& self)
	{

		TryDetach(self.get<Type>());

		__super::Destroy(self);
	}

	String LEX::ObjectInfo<RE::TESForm*>::PrintString(ObjectData& a_self, std::string_view)
	{
		auto id = GetTypeID(a_self);

		RE::TESForm* self = a_self.get<Type>();

		ITypeInfo* type = IdentityManager::instance->GetTypeByID(id);

		return std::format("{}::({}<{:08X}>)", type ? type->GetName() : "Form", self ? self->GetFormEditorID() : "", self ? self->GetFormID() : 0);
	}



	bool LEX::ObjectInfo<RE::TESForm*>::CreateLiteralData(std::string_view literal, uintptr_t& hash, ObjLitCtor& ctor)
	{
		using namespace clib_util;

		auto func = [](std::string_view lit) -> Object
			{
				MakeObject((RE::TESForm*)nullptr);
				std::string a_str = std::string{ lit };

				if (const auto splitID = string::split(a_str, "::"); splitID.size() == 2) {
					const auto  formID = string::to_num<RE::FormID>(splitID[1], true);
					const auto& modName = splitID[0];
					if (auto mapper = MergeMapperPluginAPI::GetMergeMapperInterface001()) {
						const auto [mergedModName, mergedFormID] = mapper->GetNewFormID(modName.c_str(), formID);
						return RE::TESDataHandler::GetSingleton()->LookupForm(mergedFormID, mergedModName);
					}
					else {
						return RE::TESDataHandler::GetSingleton()->LookupForm(formID, modName);
					}
				}
				if (string::is_only_hex(a_str, false)) {
					auto form = RE::TESForm::LookupByID(string::to_num<RE::FormID>(a_str, true));
					return form;
				}
				if (const auto form = RE::TESForm::LookupByEditorID(a_str)) {
					return form;
				}
				return (RE::TESForm*)nullptr;
			};

		std::string str = std::string{ literal };

		if (str.starts_with("::") == true) {
			str.insert_range(str.begin(), "Skyrim.esm");
		}
		else if (string::is_only_hex(str, false)) {
			str.insert_range(str.begin(), "Skyrim.esm::");
		}

		for (auto& ch : str)
		{
			ch = (char)std::tolower(ch);
		}

		hash = std::hash<std::string>{}(str);
		ctor = func;
		return true;
	}

	TypeOffset LEX::ObjectInfo<RE::TESForm*>::GetOffsetFromArgs(const std::string_view& category, const std::span<std::string_view>& args)
	{
		switch (Hash(category))
		{
		case "FORM"_h:
			if constexpr (1)
			{
				if (args.size() != 1)
					report::compile::critical("FORM requires 1 entry");

				auto& arg = args[0];

				if (arg != magic_enum::enum_name(RE::FormType::Max) && arg != magic_enum::enum_name(ExtraForm::kTotal))
				{
					//I can make a macro for most of this
					{
						auto value = magic_enum::enum_cast<RE::FormType>(arg);

						if (value.has_value() == true) {
							return static_cast<TypeOffset>(value.value());
						}
					}

					{
						auto value = magic_enum::enum_cast<ExtraForm>(arg);

						if (value.has_value() == true) {
							return GetExtraFormOffset(value.value());
						}
					}
				}

				report::compile::error("Unknown name '{}' used", arg);

			}
			break;

		}
		return TypeID::invalid;
	}

	INITIALIZE("main_init")
	{
		SharedClient::instance->AddCompileOptions("Skyrim");

		auto& module = REL::Module::get();

		switch (module.GetRuntime())
		{
		case REL::Module::Runtime::SE:
			SharedClient::instance->AddCompileOptions("ESV_SE");
			break;

		case REL::Module::Runtime::AE:
			SharedClient::instance->AddCompileOptions("ESV_AE");
			break;

		case REL::Module::Runtime::VR:
			SharedClient::instance->AddCompileOptions("ESV_VR");
			break;

		case REL::Module::Runtime::Unknown:
			SharedClient::instance->AddCompileOptions("ESV_NA");
			break;
		}





	}




#endif

}