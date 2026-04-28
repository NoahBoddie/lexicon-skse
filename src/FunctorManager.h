#pragma once

#include "Lexicon/Engine/ParserTest.h"

#include "nlohmann/json-schema.hpp"
#include "ConditionTLS.h"

#include "CachedScript.h"

#include "Parameter.h"
#include "ParameterCode.h"

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
	
	using ConditionBase = double(RE::TESObjectREFR::*)(
		RE::TESObjectREFR* subject,
		RE::TESObjectREFR* target,
		float solution);


	using ConditionFormula = Formula<ConditionBase>;

	template <size_t Sz>
	using FunctorFormula = Formula<typename LoadFunctionWithArgs<ConditionBase, runtime_type, Sz, false>::type>;





	struct Property
	{
		inline static std::set<Variable, var_cmp> propertyValues;

		//I think the type here might not be needed.
		std::string type{};
		std::variant<std::monostate, PropertyFormula, const Variable*> _value;
		uint32_t ownerID = -1;
		bool errorDisplayed = false;
		Property() = default;
		Property(std::string type_name, std::string formula) : type{ type_name }
		{
			auto result = PropertyFormula::Create(type_name, formula);
			_value = result;
		}

		Property(std::string type_name, Variable var) : type{ type_name }
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





		bool Load(uint32_t index)
		{
			if (this)
			{
				if (index < 0)
				{
					auto arg = value();

					//TODO: Should try to convert
					if (arg && arg->IsNumber() == true) {
						currentParams->TrySolution(arg->AsNumber());
					}
				}
				else if (index > 0) {
					index--;
					auto arg = value();
					if (arg) {
						if (currentParams->LoadArgument(index, *arg) == true)
							return true;
					}
				}
			}

			return false;
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


	/////////////////////////////////////




	template <size_t Size, size_t... Indices>
	struct FunctorHelper : public FunctorHelper<Size - 1, Indices..., sizeof...(Indices)>
	{

	};

	template <size_t... Indices>
	struct FunctorHelper<0, Indices...>
	{
		static constexpr size_t SIZE = sizeof...(Indices);

		//this should probably use an out
		static auto Execute(DynamicFormula& base, std::span<Variable> args, RE::TESObjectREFR* refr, RE::TESObjectREFR* subject, RE::TESObjectREFR* target, float solution)
		{
			double def = NAN;

			auto& formula = base.As<FunctorFormula<SIZE>>();


			return formula(refr)->Call(subject, target, solution, args[Indices]..., def);
		}

		static auto Create(std::string_view name, IScript* script, SyntaxRecord& to_process, std::span<std::pair<std::string_view, std::string_view>> params)
		{
			return FunctorFormula<SIZE>::Create(
				"target",
				"subject",
				"solution",
				params[Indices]...,
				name,
				to_process,
				script);

		}
	};


	struct Functor
	{
		enum Flags
		{
			kNone = 0,
			kInvalid = 1 << 0,
			kErrorDisplayed = 1 << 1,
			kIsSolvable = 1 << 2,
			kUsesDefault = 1 << 3,
		};



		//std::vector<Param> parameters;

		std::unique_ptr<Parameter[]> parameters = nullptr;
		uint32_t size = 0;

		//If the number of args are less than the size of parameters, but more than the minimum, it won't pad the arg 
		// collection. If the number of args is below the minimum it will fail to execute.
		uint32_t min = -1;

		//Want to make a convert function for formula handlers.
		DynamicFormula formula{};

		//Instead of default I'll make a struct that will allow me to just force the value to be one or the other.
		float defaultValue = NAN;
		Flags flags = kNone;

		void SetFlags(Flags flag, bool value)
		{
			if (value)
				flags |= flag;
			else
				flags &= flag;
		}

		void SetDefault(float def)
		{
			defaultValue = def;
			SetFlags(kUsesDefault, true);
		}

		void MarkInvalid()
		{
			SetFlags(kInvalid, true);
		}


		bool IsSolvable() const
		{
			return flags & kIsSolvable;
		}

		void SetParameters(std::span<Parameter> params)
		{
			auto length = params.size();

			if (length) {
				parameters = std::make_unique<Parameter[]>(length);
				size = length;
				std::move(params.begin(), params.end(), parameters.get());
			}
		}

		bool CreateFormula(std::string_view name, IScript* script, SyntaxRecord& to_process, std::span<std::pair<std::string_view, std::string_view>> params)
		{
			auto length = params.size();

			if (size != length) {
				//log a failure
				return false;
			}

			BinarySearch<Parameter::MAX_SIZE>(length, [&]<size_t I>
			{
				formula = FunctorHelper<I>::Create(name, script, to_process, params);
			});

			return formula;
		}

		double Execute(RE::TESObjectREFR* refr, RE::TESObjectREFR* subject, RE::TESObjectREFR* target, float solution)
		{
			//I'd like to pop regardless
			//Variable arg = Property::PopArgument();

			//This should be gotten from a function that gets a range of what youre allowed to use

			if (this) {
				std::vector<Variable> args;


				if (currentParams->PopArguments(min, size, args) == false) {
					return -1;
				}

				auto def = -1.0;

				auto length = args.size();

				for (int64_t i = 0; i < length; i++)
				{
					auto& arg = args[i];
					auto& param = parameters[i];
					auto type = param.type;

					bool is_void = arg.IsVoid();

					if (is_void && type) {
						if (param.IsOptional() == false)
							return def;

						arg = type->GetDefault();
					}
					else if (!arg.IsVoid() && !type) {
						return def;
					}

					auto arg_type = arg.GetTypeInfo();

					if (type && arg_type != type && type->Convert(arg, arg) == false) {
						return def;
					}

				}

				return BinarySearch<Parameter::MAX_SIZE>(length, [&]<size_t I>
				{
					return FunctorHelper<I>::Execute(formula, args, refr, subject, target, solution);
				});
			}
			return std::numeric_limits<double>::quiet_NaN();
		}
	};




	///////////////////////////////////////////////






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
				ConditionFormula& formula = formulas[form] = ConditionFormula::Create("subject", "target", "solution", form, cached_script::condition());

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



						std::vector<Parameter> params;
						
						std::vector<std::pair<std::string_view, std::string_view>> names;
						
						bool stop = false;

						VisitJsonIfFind(item, "parameters", [&](json& it)
							{
								if (stop) return;

								Parameter param;
								std::pair<std::string_view, std::string_view> name;

								name.first = it["type"];
								name.second = it["name"];


								IfFind(item, "optional", [&](json& it)
								{
									if (it == true)
										param.settings = Parameter::kOptional;
								});

								ITypeInfo* type = script->GetTypeFromPath(name.first);

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
								params.push_back(std::move(param));
								names.push_back(std::move(name));


							});
						


						///////////////

						//functor.ownerID = fileID;

						SyntaxRecord to_process;

						std::string_view formula = item["formula"];
						

						functor.SetFlags(Functor::kIsSolvable, FindOr(item, "isSolvable", true));

						if (!script) {
							script = cached_script::condition();
						}

						IfFind(item, "default", [&](json& number)
						{
							functor.SetDefault(static_cast<float>(number));
						});

						if (Parser::CreateSyntax<LineParser>(to_process, formula) == false) {
							logger::error("Functor formula failed to be parsed: {}", formula);
						}
						
						functor.SetParameters(params);

						if (functor.CreateFormula(name, script, to_process, names) == false) {
							logger::error("Functor formula failed to compile: {}", formula);
						}
						
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

		static constexpr ParameterCode k_loadPropCode = 0xDEAD4EAD;
		static constexpr ParameterCode k_exFuncCode = 0x600DFEED;
		static constexpr ParameterCode k_argFuncCode = 0x1BADD00D;
		static constexpr ParameterCode k_loadFileCode = 0xBAD4EED;
		static constexpr ParameterCode k_useFormulaCode = 0xDEADBEEF;

		static void HandleProperty(const std::string_view& name, void*& arg1, void*& arg2)
		{


			std::string prop_name;
			prop_name = name.substr(k_loadProperty.size());

			currentParams->AddFilename(prop_name);
			Property* property = FindProperty(prop_name);

			logger::info("Search for property {}: {}", prop_name, !!property);
			
		}

		static void HandleFunctor(const std::string_view& name, void*& arg1, void*& arg2)
		{
			std::string func_name;
			func_name = name;

			currentParams->AddFilename(func_name);
			currentParams->ClearFilename();

			Functor* functor = FindFunctor(func_name);
			logger::info("Search for functor {}: {}", func_name, !!functor);
			arg1 = functor;
			arg2 = k_exFuncCode;
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
			arg2 = k_loadFileCode;

		}


		//This is used
		static void HandleDestination(void*& packed, RE::CONDITION_ITEM_DATA& data)
		{
			if (data.flags.global) {
				return;
			}

			ParameterCode code = packed;

			int index = data.comparisonValue.f;

			packed = code.InsertData(data.comparisonValue.f);

			data.flags.global = false;
			data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
			data.comparisonValue.f = -std::numeric_limits<float>::infinity();
	
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
				arg2 = k_loadPropCode;

				HandleDestination(arg2, data);
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
				HandleDestination(arg2, data);
			}
			else if (name.starts_with(k_argueFunctor) == true) {
				HandleFunctor(name.substr(k_argueFunctor.size()), arg1, arg2);
				arg2 = k_argFuncCode;
				HandleDestination(arg2, data);
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