#pragma once

#include "Lexicon/Engine/ParserTest.h"

#include "nlohmann/json-schema.hpp"
#include "ConditionTLS.h"

#include "CachedScript.h"

namespace RE
{
	using FunctionID = FUNCTION_DATA::FunctionID;
}

namespace LEX
{
	constexpr std::string_view init_chars = "RUN";

	constexpr auto init_size = init_chars.size();
}

using nlohmann::json;
using nlohmann::json_schema::json_validator;


namespace LEX
{
	constexpr std::string_view scriptedConfig = "LEX_";

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


	using PropertyFormula = Formula<runtime_type()>;


	using ConsoleFormula = Formula<Voidable(RE::TESObjectREFR::*)(Voidable)>;
	
	//TODO: Hunt these down and delete them.
	//using ConditionFormula = Formula<double(RE::TESObjectREFR::*)(RE::TESObjectREFR*, RE::TESObjectREFR*)>;
	//using FunctorFormula = Formula<double(RE::TESObjectREFR::*)(runtime_type)>;

	using ConditionFormula = Formula<double(RE::TESObjectREFR::*)(
		runtime_type arg,
		RE::TESObjectREFR* subject, 
		RE::TESObjectREFR* target,
		float solution)>;


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
			bool result;
			

			if (str.ends_with("()") == true) {
				type = str.substr(0, str.size() - 2);
				result = true;
			}
			else {
				type = str;
				result = false;
			}

			return result;
		}

		void SetValue(Variable&& var)
		{
			auto it = propertyValues.emplace(std::move(var)).first;
			_value = &(*it);
		}


		bool SetFormula(const std::string_view& str, IScript* script)
		{
			if (!script) {
				script = cached_script::condition();
			}

			auto formula = PropertyFormula::Create(type, str, script);

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
		struct Params
		{
			static constexpr size_t maxCount = 3;

			TypeInfo* type[maxCount];
			bool optional = false;
		};

		ConditionFormula formula{};




		//I may make this a multi parameter set up by making it a null terminated unique_ptr string of type infos.
		TypeInfo* parameter = nullptr;

		std::optional<float> defaultValue = std::nullopt;
		uint32_t ownerID = -1;
		bool errorDisplayed = false;//Done so we can skip over stuff like the thing not loading.
		bool isSolvable = true;
		bool optionalParam = false;
		
		double GetDefault()
		{
			return defaultValue.value_or(0);
		}



		double Execute(RE::TESObjectREFR* refr, RE::TESObjectREFR* subject, RE::TESObjectREFR* target, float solution)
		{
			//I'd like to pop regardless
			Variable arg = Property::PopArgument();

			if (this) {
				auto def = GetDefault();

				bool is_void = arg.IsVoid();

				if (is_void && parameter) {
					if (!optionalParam)
						return def;

					arg = parameter->GetDefault();
				}
				else if (!arg.IsVoid() && !parameter) {
					return def;
				}

				auto arg_type = arg.GetTypeInfo();
				//This needs to use convert eventually.
				if (!parameter || arg_type == parameter || parameter->Convert(arg, arg) == true) {
					return formula(refr)->Call(arg, subject, target, solution, def);
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


		//This is a set of loose formulas, in case someone copies and pastes them. It will also allow 
		// me to not delete them. when deleting a condition
		inline static std::map<std::string, ConditionFormula> formulas;

		//I'm thinking of giving these ids instead so I can insert them somewhere and forget about them.
		inline static std::map<std::string, Property, std::less<>> propertyNames;//Might use reference wrappers
		inline static std::map<std::string, Functor, std::less<>> functors;

		//Used to hold onto strings that are used to load file names to resolve ambiguity
		inline static std::set<std::string> preservedNames{};

		//This is what should be in the condition function thingy majig.
		//external Variable GetProperty(string name);



		static ConditionFormula* CreateFormula(std::string_view str)
		{
			std::string form;
			form = str;

			report::compile::info("compiling '{}'", form);


			auto it = formulas.find(form);


			ConditionFormula* result;

			if (formulas.end() == it) {
				ConditionFormula& formula = formulas[form] = ConditionFormula::Create({ "voidable", "arg" }, "subject", "target", "solution", form, cached_script::condition());

				if (formula) {
					report::compile::info("Successfully compiled '{}'", form);
				}
				else {
					report::compile::failure("Condition [{}] failed to compile.", form);
				}

				result = &formula;
			}
			else
			{
				ConditionFormula& formula = formulas[form];
				
				if (formula) {
					report::compile::info("Reacquired successful formula [{}]", form);
				}
				else {
					report::compile::failure("Reacquired failed formula [{}]", form);
				}

				result = &formula;
			}



			return result;
		}



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
		static void LoadProperties(uint32_t fileID, std::string_view filename, json& settings, IScript* script)
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

						bool is_formula = property.SetType(item["type"]);
						
						property.ownerID = fileID;

						auto& value = item["value"];
						
						bool commit = true;

						if (is_formula && value.type() != json::value_t::string) {
							logger::warn("Expected property {}::{} to be a string", filename, name);
						}

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

									if (is_formula) {
										commit = property.SetFormula(str, script);
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

		static void LoadFunctors(uint32_t fileID, std::string_view filename, json& settings, IScript* script)
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

						bool has_param;

						if (IfFind(item, "parameter", [&](json& type)
							{
								has_param = true;
								parameter = type;
							}) == false)
						{
							has_param = false;
							parameter = "voidable";
						}

						functor.isSolvable = FindOr(item, "isSolvable", true);

						if (!script) {
							script = cached_script::condition();
						}

						if (has_param)
						{
							ITypeInfo* type = script->GetTypeFromPath(parameter);

							if (!type) {
								logger::error("Type for parameter cannot be found: {}", parameter);
							}


							functor.parameter = type->GetTypeInfo(nullptr);

							if (!functor.parameter) {
								logger::error("Type for parameter is not complete: {}", parameter);
							}
						}
						
						IfFind(item, "default", [&](json& number)
							{
								functor.defaultValue = static_cast<float>(number);
							});

						IfFind(item, "optional", [&](json& value)
							{
								functor.optionalParam = value;
							});



						if (Parser::CreateSyntax<LineParser>(to_process, formula) == false) {
							logger::error("Functor formula failed to be parsed: {}", formula);
						}

						//Handle process here

						auto func_form = ConditionFormula::Create(
							{ parameter, "arg"}, 
							"target", 
							"subject",
							"solution",
							name, 
							to_process, 
							script);

						if (!func_form) {
							logger::error("Functor formula failed to compile: {}", formula);
						}
						//Check validation here.

						functor.formula = func_form;
						
						functors[name] = std::move(functor);
					}
				});
		}

		static bool LoadFile(std::string_view filename, std::string_view text, IScript* script) try
		{
			json contents = json::parse(text, nullptr, true, true);

			logger::debug("{}:\n{}", filename, contents.dump(2));

			//Toss the json bit.
			filename = filename.substr(0, filename.size() - 5);

			LoadSettings(filename, contents);

			uint32_t filecode = nextFileCode++;

			LoadProperties(filecode, filename, contents, script);
			LoadFunctors(filecode, filename, contents, script);
			return true;
		} catch (std::exception& error)
		{
			logger::error("Error for {}: {}", filename, error.what());
			return false;
		}
		


		static void LoadFiles()
		{
			//This should use the selected lexicon folder
			std::vector<std::pair<std::string, std::string>> files =  SearchFiles("Data/SKSE/Lexicon/Resources/LexiconSKSE", ".json");
			auto script = cached_script::condition();

			for (auto& [folder, filename] : files)
			{
				if (filename.starts_with(scriptedConfig) == true) {
					logger::warn("Config '{}' cannot be loaded via file ('{}' suffix is reserved for script formatting)", 
						filename, scriptedConfig);
				}

				json contents;

				try
				{
					std::string path = std::format("{}/{}", folder, filename);
					logger::info("loading: {}", path);
					std::ifstream file_input{ path };
					std::stringstream stream;
					stream << file_input.rdbuf();
					std::string text = stream.str();

					LoadFile(filename, text, script);
				}
				catch (std::exception& error)
				{
					logger::error("Error for {}: {}", filename, error.what());
					continue;
				}

			}
		}




		static void Initialize()
		{
			if (_init)
				return;
		
		
			LoadFiles();
			//I'll be frank, I fucking hate this as a concept. I think it's bad as shit.
			// With that being said, this seems to be the safest place to do this for now.
			// But ideally, I'd like to execute this around when the game finishes it's static initialization

			_init = true;
			LoadWaiting();

			

		}

		static constexpr std::string_view k_loadProperty = "PROPERTY__";
		static constexpr std::string_view k_executeFunctor = "FUNCTOR__";
		static constexpr std::string_view k_argueFunctor = "ARGFUNCT__";
		static constexpr std::string_view k_returnFunctor = "RETURN__";
		static constexpr std::string_view k_loadFile = "LOADFILE__";
		static constexpr std::string_view k_setFile = "SETFILE__";

		inline static uint32_t nextFileCode = 1;

		//LOADPROP, CALLFUNC, LOADFILE are the names I'll be using.

		static constexpr size_t k_loadPropCode = 0xDEAD4EAD;
		static constexpr size_t k_exFuncCode = 0x600DFEED;
		static constexpr size_t k_argFuncCode = 0x1BADD00D;
		static constexpr size_t k_loadFileCode = 0xBAD4EED;

		static void HandleFunctor(const std::string_view& name, void*& arg1, void*& arg2)
		{
			std::string func_name;
			func_name = name;

			currentParams->AddFilename(func_name);
			currentParams->ClearFilename();

			Functor* functor = FindFunctor(func_name);
			logger::info("Search for functor {}: {}", func_name, !!functor);
			arg1 = functor;
			arg2 = reinterpret_cast<void*>(k_exFuncCode);
		}

		static void HandleFile(const std::string_view& name, void*& arg1, void*& arg2, bool lock)
		{
			auto it = preservedNames.find(std::string{ name });

			bool found = preservedNames.end() != it;

			if (!found) {
				logger::warn("Loaded file is not required for anything: {}", name);
			}
			else {
				currentParams->SetFilename(*it, !lock);
			}
			arg1 = reinterpret_cast<void*>(found);
			arg2 = reinterpret_cast<void*>(k_loadFileCode);

		}


		static void AutoPassCheck(RE::CONDITION_ITEM_DATA& data)
		{
			if (data.flags.global && data.comparisonValue.g) {
				std::string_view name = data.comparisonValue.g->GetFormEditorID();

				if (name == "LEX_FORCE_SUCCESS") {
					data.flags.global = false;
					data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo;
					data.comparisonValue.f = -std::numeric_limits<float>::infinity();
				}
			}
		}

		static bool Apply(RE::BGSKeyword* keyword, void*& arg1, void*& arg2, RE::CONDITION_ITEM_DATA& data)
		{
			if (!keyword)
				return false;




			std::string_view name = keyword->formEditorID;


			bool result = true;

			if (name.starts_with(k_loadProperty) == true) {
				std::string prop_name;
				prop_name = name.substr(k_loadProperty.size());

				currentParams->AddFilename(prop_name);

				Property* property = FindProperty(prop_name);

				logger::info("Search for property {}: {}", prop_name, !!property);
				arg1 = property;
				arg2 = reinterpret_cast<void*>(k_loadPropCode);
			}
			else if (name.starts_with(k_executeFunctor) == true) {
				HandleFunctor(name.substr(k_executeFunctor.size()), arg1, arg2);

				if (!arg1) {
					data.flags.global = false;
					data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
					data.comparisonValue.f = -std::numeric_limits<float>::infinity();
				}
			}
			else if (name.starts_with(k_returnFunctor) == true) {
				HandleFunctor(name.substr(k_returnFunctor.size()), arg1, arg2);
				data.flags.global = false;
				data.flags.opCode = arg1 ? RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo : RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
				data.comparisonValue.f = -std::numeric_limits<float>::infinity();
			}
			else if (name.starts_with(k_argueFunctor) == true) {
				HandleFunctor(name.substr(k_argueFunctor.size()), arg1, arg2);
				arg2 = reinterpret_cast<void*>(k_argFuncCode);
				data.flags.global = false;
				data.flags.opCode = arg1 ? RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo : RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
				data.comparisonValue.f = -std::numeric_limits<float>::infinity();
			}
			else if (name.starts_with(k_loadFile) == true) {
				HandleFile(name.substr(k_loadFile.size()), arg1, arg2, false);
			}
			else if (name.starts_with(k_setFile) == true) {
				HandleFile(name.substr(k_setFile.size()), arg1, arg2, true);
			}
			else {
				result = false;
			}

			if (result) {
				AutoPassCheck(data);
			}


			return result;
		}
		
		struct File
		{
			std::string filename;
			std::string content;
			IScript* script;


		};


		inline static std::vector<File> waitingFiles;
		
		static void LoadWaiting()
		{
			for (auto& waiter : waitingFiles){
				FunctorManager::LoadFile(waiter.filename, waiter.content, waiter.script);
			}

			waitingFiles.clear();
		}

		static void HoldFile(std::string filename, std::string content, IScript* script)
		{
			if (_init) {
				FunctorManager::LoadFile(filename, content, script);
				return;
			}

			waitingFiles.emplace_back(filename, content, script);
		}


		inline static bool _init = false;

	};

	INITIALIZE("main_init")
	{
		SharedClient::instance->AddCompileOptions("Skyrim");
		//Want to add a format that loads a script like above. Anything loaded through through script will have
		// a header, and that header will be disallowed.

		SharedClient::instance->AddFormatter("Condition", [](
			IScript* script,
			const std::string_view& format,
			const std::string_view& content)
			{
				std::string filename = std::format("{}{}", scriptedConfig, script->GetName());
				clib_util::string::replace_all(filename, "::", "__");
				FunctorManager::HoldFile(filename, std::string{ content }, script);
				return true;
				//return singleton->tmpLoadFile(filename, content, script);
			});



	}

}