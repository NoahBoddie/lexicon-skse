#pragma once

#include "Lexicon/AttributeData.h"
#include "Lexicon/Interfaces/AttributeManager.h"
#include "Lexicon/Interfaces/ProcedureHandler.h"
#include "Lexicon/Utility/Parameter.h"
#include "FunctorManager.h"
namespace LEX
{
	struct ConditionAttribute : public AttributeData
	{
		constexpr static char ATTRIBUTE_NAME[] = "Shared::Commons::ConditionAttribute";
		//Currently due to my laziness, this isn't required.
		//static void CondtionAttribute_Ctor(Util::Attribute<"Shared::Commons::ConditionAttribute">&& a_this, float def, bool use_file){}


		//T
		static void Constructor(Util::Attribute<ATTRIBUTE_NAME>&& a_this, float def, bool use_file)
		{
			//TODO: make this a function in attribute base.
			ConditionAttribute* self = dynamic_cast<ConditionAttribute*>(a_this->GetNativeData());
			
			assert(self);
			//Include required 
			self->defValue = def;
			self->usesFile = use_file;
		}


		bool CanAllowAttach(AttributeOwner* owner) override 
		{ 
			IComponent* component = owner->AsOwner<IComponent>();

			ConcreteFunction* function = component->As<ConcreteFunction>();

			if (!function) {
				report::compile::failure("ConditionAttribute can only attach to a non-generic function");
				return false;
			}

			if (function->HasVadiadic() == true) {
				report::compile::failure("ConditionAttribute can only attach to a non-variadic function");
				return false;
			}


			auto return_type = function->GetReturnType();

			if (return_type->CanConvertTo(common_type::double_t()) == false) {
				report::compile::failure("ConditionAttribute can only attach to a function with a return type convertible to a float");
				return false;
			}

			auto this_type = function->GetTargetType();

			if (!this_type) {
				report::compile::failure("ConditionAttribute can only attach to a function with a return type convertible to a float");
				return false;
			}

			TypeInfo* obj_ref = common_type::get_from_type<RE::TESObjectREFR*>();
			TypeInfo* bnd_obj = common_type::get_from_type<RE::TESBoundObject*>();

			
			//TODO: please make this something like common_type
			if (this_type->IsDerivedFrom(obj_ref) == false && 
				this_type->IsDerivedFrom(bnd_obj) == false) {
				report::compile::failure("ConditionAttribute can only attach to a function that targets a type deriving from ObjectReference");
				return false;
			}

			return true;
		}

		//These should be constructing a functor as they go along.
		
		//This string should include the member access
		std::string CreateExecution(ConcreteFunction* function, std::vector<Parameter>& params, std::vector<std::string_view>& names, std::string_view this_string = "")
		{
			std::string execute_string;

			if (this_string.empty() == false) {
				execute_string = std::format("{}.", this_string);
			}

			execute_string += function->GetName();
			execute_string += '(';

			
			//IMPORTANT: I'd like to make it so uses of the parameter names __solution__, __target__, and __subject__ won't
			// create a new parameter. __solution__ needs to be float convertible. target and subject will also need to be.
			for (auto& param : function->parameters) {
				auto index = param.GetIndex();
				
				if (index > 1) {
					execute_string += ", ";
				}
				
				std::string_view name = param.GetName();

				execute_string += param.GetName();

				//This will not be supported for a while, I'd like to get base concepts off the ground.
				if (index == function->vardIndex) {
					execute_string += "...";
				}

				Parameter parameter{};
				
				parameter.type = param.GetType()->As<TypeInfo>();

				//parameter.settings = Parameter::kOptional;
				
				/*
				if (!type) {
					logger::error("Type for parameter cannot be found: {}", name.first);
					stop = true;
					return;
				}

				param.type = type->GetTypeInfo(nullptr);

				if (!param.type) {
					logger::error("Type for parameter is not complete: {}", name.first);
					stop = true;
					return;
				}
				//*/

				assert(parameter.type);
				
				TypeInfo* expected_type;

				switch (Hash(name))
				{
				case "target"_h:
				case "subject"_h:
					expected_type = common_type::get_from_type<RE::TESObjectREFR*>();
					break;
				case "solution"_h:
					expected_type = common_type::floating();
					break;

				default:
					expected_type = nullptr;
					break;
				}

				if (expected_type && expected_type != parameter.type) {
					report::compile::error("reserved name '{}' didn't have expected type '{}'", name, expected_type->GetName());
				}

				if (!expected_type) {
					params.push_back(parameter);
					names.push_back(name);
				}
				


			}

			execute_string += ')';

			return execute_string;
			//*/
		}


		void OnTargetValidated() override
		{
			IComponent* component = GetParent()->AsOwner<IComponent>();

			ConcreteFunction* function = component->As<ConcreteFunction>();


			//After this point, make the lions share of what's found here a function.

			Script* script = function->GetScript();




			std::string name; 

			if (usesFile) {
				name = std::format("{}@{}", function->GetName(), script->GetName());
			}
			else {
				name = function->GetName();
			}


			//if (functors.find(name) != functors.end()) {
			//	logger::error("Functor name {} already exists.", key);
			//	continue;
			//}



			//For now, I'm going to focus on generating the string.
			//Parameter names will all match, and the this will be measured for being object reference. If it's not, it it will attempt to upcast to it.
			// Another thing though, if the object is a bound object, It will attempt to get the base object, and cast it into the type.
			//I'll save this bit for later, as well for more complex things.


			
			Functor functor{};
			functor.SetFlags(Functor::kIsSolvable, isSolvable);
			functor.SetDefault(defValue);



			std::string formula_string;

			//*


			auto this_type = function->GetTargetType();


			std::vector<Parameter> params;
			std::vector<std::string_view> names;
			

			if (TypeInfo* obj_ref = common_type::get_from_type<RE::TESObjectREFR*>(); obj_ref != this_type) {
				
				std::string type_string = this_type->GetFullName();
				
				std::string_view format_string;
				
				formula_string += '{';

				if (this_type->IsDerivedFrom(obj_ref) == true) {
					formula_string += std::format(
					R"(
					{} a_this = this as {};
					if (a_this) return {};
					return {};
					)", 
					type_string,
					type_string,
					CreateExecution(function, params, names, "a_this"),
					0.f);
				}
				else {//TypeInfo* bnd_obj = common_type::get_from_type<RE::TESBoundObject*>();
					formula_string += std::format(
						R"(
						{} a_base = GetBaseObject() as {};
						if (a_base) return {};
						return {};
						)",
						type_string,
						type_string,
						CreateExecution(function, params, names, "a_base"),
						0.f);
				}

				formula_string += '}';

			}
			else {
				formula_string = CreateExecution(function, params, names);
			}
			
			FunctorManager;
			logger::debug("formula for {}:\n{}", function->GetName(), formula_string);			
			
			//return;

			SyntaxRecord to_process;

			//if (!script) {
			//	script = cached_script::condition();
			//}


			if (Parser::CreateSyntax<LineParser>(to_process, formula_string) == false) {
				logger::error("Functor formula failed to be parsed:\n{}", formula_string);
			}

			
			std::vector<std::pair<FormulaParam, std::string_view>> param_names{ params.size() };

			for (int i = 0; i < params.size(); i++){
				param_names[i] = { params[i].type, names[i] };
			}
			
			functor.SetParameters(params);

			if (functor.CreateFormula(name, script, to_process, param_names) == false) {
				report::compile::error("Functor formula failed to compile: {}", formula_string);
			}

			FunctorManager::functors[name] = std::move(functor);


			report::compile::debug("Functor Complete: {}", formula_string);
		}



		
		//This is my way to solve not having optionals (or I could just make optionals.
		uint8_t paramsRequired = -1;
		bool isSolvable = true;
		bool usesFile = true;
		double defValue = NAN;
	};

	INITIALIZE("main_init")
	{
		AttributeManager::instance->RegisterNativeData<ConditionAttribute>();
		//ProcedureHandler::instance->RegisterConstructor(ConditionAttribute::Constructor, ConditionAttribute::ATTRIBUTE_NAME);

	}
}