#pragma once

#include "Lexicon/Engine/ParserTest.h"

#include "nlohmann/json-schema.hpp"
#include "ConditionTLS.h"

namespace LEX
{

}


using nlohmann::json;
using nlohmann::json_schema::json_validator;


namespace LEX
{


	inline void VisitJsonArray(json& value, std::function<void(json&)> func)
	{
		//Add required size

		bool is_array = value.is_array();

		auto size = is_array ? value.size() : 1;


		for (int i = 0; i < size; i++)
		{
			auto& entry = is_array ? value[i] : value;

			func(entry);
		}
	}


	template <typename T>
	T FindOr(json& ref, const std::string_view& name, T def)
	{
		auto it = ref.find(name);

		if (ref.end() == it)
			return def;
		else
			return it.value();
	}


	inline bool IfFind(json& ref, const std::string_view& name, std::function<void(json::iterator&)> func)
	{
		auto it = ref.find(name);

		auto find = ref.end() != it;

		if (find)
			func(it);

		return find;
	}


	inline bool IfFind(json& ref, const std::string_view& name, std::function<void(json&)> func)
	{
		auto it = ref.find(name);

		auto find = ref.end() != it;

		if (find)
			func(it.value());

		return find;
	}


	inline bool VisitJsonIfFind(json& ref, const std::string_view& name, std::function<void(json&)> func)
	{
		return IfFind(ref, name, [&](json& found) { VisitJsonArray(found, func); });
	}


	using FunctorFormula = Formula<double(RE::TESObjectREFR::*)(runtime_type)>;
	using PropertyFormula = Formula<runtime_type()>;

	void TestingFact()
	{
	}

	struct Property
	{
		inline static std::set<Variable, var_cmp> propertyValues;
		
		std::string type{};
		std::variant<std::monostate, PropertyFormula, const Variable*> _value;
		uint32_t ownerID = -1;
		bool errorDisplayed = false;
		Property() = default;
		Property(std::string type_name, std::string formula) : type { type_name }
		{
			auto result = PropertyFormula::Create(type_name, formula);
			_value = result;
		}

		Property(std::string type_name, Variable var) : type { type_name }
		{
			auto it = propertyValues.emplace(std::move(var)).first;
			_value = &(*it);
			//_value = &(*propertyValues.emplace(std::move(var)).first);
			logger::info("index {}", _value.index());
		}

		bool SetType(const std::string_view& str)
		{
			type = str;

			return true;
		}

		void SetValue(Variable&& var)
		{
			auto it = propertyValues.emplace(std::move(var)).first;
			_value = &(*it);
		}


		bool SetFormula(const std::string_view& str)
		{
			auto formula = PropertyFormula::Create(type, str);

			if (formula) {
				_value = std::move(formula);
				return true;
			}

			return false;
		}



		static bool LoadArgument(const Variable& arg)
		{
			if (!arg.IsVoid()) {
				
				//TODO: I'm actually willing to have this push out, but perhaps give a warning if it does.
				return currentParams->SetArgument(arg);
			}

			return false;
		}

		 
		static Variable PopArgument()
		{
			auto result = currentParams->GetArgument();;
			currentParams->ClearArgument();

			return result;
		}

		double LoadAsArgument()
		{
			if (this)
			{
				auto arg = value();
				if (arg) {
					if (currentParams->SetArgument(*arg) == true)
					return 1.0;
				}
			}

			return -1.0;
		}

	
		

		const Variable* value()
		{
			const Variable* result = nullptr;

			size_t index = _value.index();
			if (index == 1) {
				if (1)//if data loaded has finished.
				{
					auto& formula = std::get<PropertyFormula>(_value);

					auto var = formula(Void{});

					if (!var.IsVoid()) {
						auto it = propertyValues.emplace(std::move(var)).first;
						result = &(*it);
					}


					_value = result;
				}
			}
			else if (index == 2) {
				result = std::get<const Variable*>(_value);
			}

			return result;
		}

		bool IsValid() const
		{

		}


	};

	struct Functor
	{
		FunctorFormula formula{};

		//I may make this a multi parameter set up by making it a null terminated unique_ptr string of type infos.
		TypeInfo* parameter = nullptr;

		std::optional<float> defaultValue = std::nullopt;
		uint32_t ownerID = -1;
		bool errorDisplayed = false;//Done so we can skip over stuff like the thing not loading.

		double GetDefault()
		{
			constexpr double nan = std::numeric_limits<double>::quiet_NaN();
			return defaultValue.value_or(nan);
		}



		double Execute(RE::TESObjectREFR* refr)
		{
			//I'd like to pop regardless
			Variable arg = Property::PopArgument();

			if (this) {
				constexpr double nan = std::numeric_limits<double>::quiet_NaN();

				if (arg.IsVoid() && parameter) {
					return nan;
				}

				if (!arg.IsVoid() && !parameter) {
					return nan;
				}

				auto arg_type = arg.GetTypeInfo();
				//This needs to use convert eventually.
				if (arg_type == parameter || parameter->Convert(arg, arg) == true) {
					return formula(refr)->Call(arg, GetDefault());
				}
				

			}
			return std::numeric_limits<double>::quiet_NaN();
		}

	};




	inline std::unordered_map<std::string, std::string> list;
	//Here's the idea I'm thinking. The left side is the name of a given property
	// the right side is what that property turned into
	//Properties marked with * mean that it needs to be created first. if it fails to generate, anything that
	// uses that will then fail as well




	struct FunctorManager
	{
		



		////////////////////////////////////




		


		static Functor* FindFunctor(const std::string_view& name)
		{
			auto it = functors.find(name);

			if (functors.end() == it)
				return nullptr;

			return &it->second;
		}


		//This whole thing is a terrible fucking idea, and should be relegated to something else some how.

		static Property* FindProperty(const std::string_view& name)
		{
			auto it = propertyNames.find(name);

			if (propertyNames.end() == it)
				return nullptr;

			return &it->second;
		}

		
		//This is a register function
		static Voidable GetProperty(const std::string_view& name)
		{
			Property* result = FindProperty(name);

			return result && result->value() ? *result->value() : Voidable{};
		}



		inline static std::map<std::string, Property, std::less<>> propertyNames;//Might use reference wrappers
		inline static std::map<std::string, Functor, std::less<>> functors;

		//Used to hold onto strings that are used to load file names to resolve ambiguity
		inline static std::set<std::string> preservedNames{};

		//This is what should be in the condition function thingy majig.
		//external Variable GetProperty(string name);




		bool InitProperty(const std::string_view& file_name, SyntaxRecord& node)
		{
			if (node.size() != 0) {
				//If it has scope names, we have no need for this sort of thing.
				return true;
			}

			Property* property = FindProperty(std::format("{}_{}", file_name, node.GetTag()));

			if (!property)
				return true;

			const Variable* value = property->value();

			//property exists but didn't compile right.
			if (!value) {
				return false;
			}

			std::string text = std::format("GetProperty('{}_{}') as {}", file_name, node.GetView(), property->type);

			if (Parser::CreateSyntax<LineParser>(node, text) == false) {
				return false;
			}

			assert_if(node.GetSyntax().type != SyntaxType::Call) {
				return false;
			}

			return true;
		}

		bool TranslateAST(const std::string_view& file_name, SyntaxRecord& ast)
		{
			for (SyntaxRecord& node : ast.children())
			{
				bool success;

				if (node.GetSyntax().type == SyntaxType::Field) {
					success = InitProperty(file_name, node);
				}
				else {
					success = TranslateAST(file_name, node);
				}

				if (!success)
					return false;
			}

			return true;
		}
		

		



		static std::string JsonName(std::string_view filename, std::string name, json& object)
		{

			IfFind(object, "requireFile", [&](json& boolean) {
				if (static_cast<bool>(boolean) == true) {
					name = std::format("{}@{}", name, filename);
					preservedNames.emplace(std::string{ filename });
				}
				});

			return name;

		}





		static void LoadSettings(std::string_view filename, json& settings)
		{
			IfFind(settings, "version", [filename](json& version)
				{
					REL::Version expected{ static_cast<std::string_view>(version) };
					REL::Version current = REL::Module::get().version();
					if (current < expected) {
						throw std::exception(std::format("Current version ({}) is lower than expected version ({}) for {}", current.string(), expected.string(), filename).c_str());
					}
				});

			VisitJsonIfFind(settings, "parents", [filename](json& parent_name)
				{
					static RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

					std::string_view name = parent_name;

					if (!handler->LookupLoadedModByName(name) && !handler->LookupLoadedLightModByName(name)) {
						throw std::exception(std::format("Required plugin {} is not enabled for {}", name, filename).c_str());
					}
				});
		}

		//I will probably include a code instead
		static void LoadProperties(uint32_t fileID, std::string_view filename, json& settings)
		{
			IfFind(settings, "properties", [&](json& props)
				{

					for (auto& [key, item] : props.items())
					{
						std::string name = JsonName(filename, key, item);

						if (propertyNames.find(name) != propertyNames.end()) {
							logger::error("Property name {} already exists.", key);
							continue;
						}


						Property property{};

						property.SetType(item["type"]);
						
						property.ownerID = fileID;

						auto& value = item["value"];
						
						bool commit = true;

						switch (value.type())
						{
							case json::value_t::number_float:
								property.SetValue(static_cast<float>(value));
								break;

							case json::value_t::number_integer:
								property.SetValue(static_cast<int32_t>(value));
								break;

							case json::value_t::number_unsigned:
								property.SetValue(static_cast<uint32_t>(value));
								break;

							case json::value_t::boolean:
								property.SetValue(static_cast<bool>(value));
								break;
							case json::value_t::string:
								if constexpr (1)
								{
									std::string_view str = static_cast<std::string_view>(value);

									if (property.type == "string" && str.starts_with(">>")) {
										commit = property.SetFormula(str.substr(2));
									}
									else {
										property.SetValue(str);
									}
									
								}
								break;

							default:
								logger::error("invalid json type for property value detected: {}", magic_enum::enum_name(value.type()));
								commit = false;
								break;
						}

						if (commit) {
							propertyNames[name] = std::move(property);
						}

					}
				});
		}

		static void LoadFunctors(uint32_t fileID, std::string_view filename, json& settings)
		{

			IfFind(settings, "functions", [&](json& props)
				{
					for (auto& [key, item] : props.items())
					{
						std::string name = JsonName(filename, key, item);


						if (functors.find(name) != functors.end()) {
							logger::error("Functor name {} already exists.", key);
							continue;
						}

						Functor functor{};

						functor.ownerID = fileID;

						SyntaxRecord to_process;

						std::string_view formula = item["formula"];
						
						std::string_view parameter;
						//TODO: confirm this type exists.



						if (IfFind(item, "parameter", [&](json& type)
							{
								parameter = type;
							}) == false)
						{
							parameter = "voidable";
						}

						functor.parameter = NULL_OP(NULL_Q(ProjectManager::instance->GetTypeFromPath(parameter))->GetTypeInfo(nullptr));

						if (!functor.parameter) {
							logger::error("invalid type for parameter: {}", parameter);
						}

						IfFind(item, "default", [&](json& number)
							{
								functor.defaultValue = static_cast<float>(number);
							});

						if (Parser::CreateSyntax<LineParser>(to_process, formula) == false) {
							logger::error("Functor formula failed to be parsed: {}", formula);
						}

						//Handle process here

						auto func_form = FunctorFormula::Create({ parameter, "arg"}, name, to_process);

						if (!func_form) {
							logger::error("Functor formula failed to compile: {}", formula);
						}
						//Check validation here.

						functor.formula = func_form;
						
						functors[name] = std::move(functor);
					}
				});
		}

		inline static uint32_t nextFileCode = 1;

		void tmpLoadFiles()
		{
			//This should use the selected lexicon folder
			std::vector<std::pair<std::string, std::string>> files =  SearchFiles("Data/SKSE/Lexicon/Resources/LexiconSKSE", ".json");


			for (auto& [folder, filename] : files)
			{
				json contents;

				try
				{
					std::string path = std::format("{}/{}", folder, filename);
					logger::info("loading: {}", path);
					std::ifstream f{ path };
					contents = json::parse(f, nullptr, true, true);

					logger::debug("{}", contents.dump(2));

					//Toss the json bit.
					filename = filename.substr(0, filename.size() - 5);

					LoadSettings(filename, contents);

					uint32_t filecode = nextFileCode++;

					LoadProperties(filecode, filename, contents);
					LoadFunctors(filecode, filename, contents);
				}
				catch (std::exception& error)
				{
					logger::error("Error for {}: {}", filename, error.what());
					continue;
				}

			}
		}




		void Initialize()
		{
			if (_init)
				return;
		
			if constexpr (0)
			{
				propertyNames["Health"] = Property("string", Variable{ "Health" });


				Functor functor{};

				functor.formula = FunctorFormula::Create({ "string", "av" }, "(this as Actor).GetActorValue(av, 15)");
				functor.defaultValue = -1.f;
				functor.parameter = common_type::string();
				functors["GetActorValue"] = std::move(functor);
			}
			else
			{
				tmpLoadFiles();
			}

			_init = true;
		}

		static FunctorManager* GetSingleton()
		{
			static FunctorManager singleton;
			singleton.Initialize();
			return &singleton;

		}

		static constexpr std::string_view k_loadProperty = "PROPERTY__";
		static constexpr std::string_view k_executeFunctor = "FUNCTOR__";
		static constexpr std::string_view k_loadFile = "LOADFILE__";

		//LOADPROP, CALLFUNC, LOADFILE are the names I'll be using.

		static constexpr size_t k_loadPropCode = 0xDEAD4EAD;
		static constexpr size_t k_exFuncCode = 0x600DFEED;
		static constexpr size_t k_loadFileCode = 0xBAD4EED;


		static bool Apply(RE::BGSKeyword* keyword, void*& arg1, void*& arg2)
		{
			if (!keyword)
				return false;

			auto key_size = keyword->formEditorID.size();

			if (auto size = k_loadProperty.size(); key_size > size &&  strnicmp(k_loadProperty.data(), keyword->GetFormEditorID(), size) == 0) {
				std::string prop_name = keyword->GetFormEditorID() + size;
				
				currentParams->AddFilename(prop_name);

				Property* property = GetSingleton()->FindProperty(prop_name);

				logger::info("Search for property {}: {}", prop_name, !!property);
				arg1 = property;
				arg2 = reinterpret_cast<void*>(k_loadPropCode);
				return true;
			}
			else if (auto size = k_executeFunctor.size(); key_size > size && strnicmp(k_executeFunctor.data(), keyword->GetFormEditorID(), size) == 0) {
				std::string func_name = keyword->GetFormEditorID() + size;

				currentParams->AddFilename(func_name);


				Functor* functor = GetSingleton()->FindFunctor(func_name);
				logger::info("Search for functor {}: {}", func_name, !!functor);
				arg1 = functor;
				arg2 = reinterpret_cast<void*>(k_exFuncCode);
				return true;
			}
			else if (auto size = k_loadFile.size(); key_size > size && strnicmp(k_loadFile.data(), keyword->GetFormEditorID(), size) == 0) {
				std::string file_name = keyword->GetFormEditorID() + size;

				auto it = preservedNames.emplace(file_name);

				if (!it.second) {
					logger::warn("Loaded file is not required for anything: {}", file_name);
				}
				else {
					currentParams->SetFilename(*it.first);
				}
				arg1 = reinterpret_cast<void*>(it.second);
				arg2 = reinterpret_cast<void*>(k_loadFileCode);

				return true;
			}


			return false;
		}
		


		inline static bool _init = false;


		

	};

	std::vector<std::pair<std::string, std::string>> stuff;



}