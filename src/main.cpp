INITIALIZE_NOW()
{
    logger::InitializeLogging();
}

#include "GameObjectStuff.h"
#include "Lexicon/Engine/TempConstruct.h"
#include "Lexicon/Interfaces/InterfaceManager.h"

#
#include "Lexicon/Engine/SettingManager.h"

#include <stacktrace>


#include <iostream>
#include <stacktrace>


#include "nlohmann/json-schema.hpp"
#include "FunctorManager.h"

#include "FunctionRegister.h"
#include "ScriptFunctions.hpp"
//#include "ConditionFunction.h"
#include "Hooks.hpp"
int nested_func(int c)
{
    std::cout << std::stacktrace::current() << '\n';
    return c + 1;
}

int func(int b)
{
    return nested_func(b + 1);
}

int main()
{
    std::cout << func(777);
}
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

//using namespace SOS;
//using namespace RGL;

using namespace LEX;



static void PrintAST(Record& tree, std::string indent = "")
{
    const static std::string __dent = "|  ";

    std::string log = tree.Print<Syntax>();

    RGL_LOG(debug, "{}{}", indent, log);

    indent += __dent;

    for (auto& child_rec : tree.children())
    {
        PrintAST(child_rec, indent);
    }
}




DEFAULT_LOGGER()
{
    auto path = log_directory();
    if (!path) {
        report_and_fail("Unable to lookup SKSE logs directory.");
    }
    *path /= PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    std::shared_ptr<spdlog::logger> log;
    if (IsDebuggerPresent()) {
        log = std::make_shared<spdlog::logger>(
            "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
    }
    else {
        log = std::make_shared<spdlog::logger>(
            "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
    }


#ifndef NDEBUG
    auto level = spdlog::level::trace;
#else
    //Use right alt for just debug logging, control to allow debugger to attach.
    auto level = GetKeyState(VK_RCONTROL) & 0x800 || GetKeyState(VK_RMENU) & 0x800 ?
        spdlog::level::debug : spdlog::level::info;
#endif

    if (level >= spdlog::level::info) {
        level = LEX::SettingManager::GetSingleton()->level;
    }


    log->set_level(level);
    log->flush_on(level);

    log->log({}, {});;

    spdlog::set_default_logger(std::move(log));


    //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);


#ifdef NDEBUG
    if (spdlog::level::debug == level) {
        logger::debug("debug logger in release enabled.");
    }
#endif

    logger::info("INITIALIZED");
}


void LogDis(std::string_view name, float value)
{
    constexpr auto  offset = LEX::Number::Settings::CreateFromType<unsigned int>().GetOffset();

    logger::info("The current health of {} is {}", name, value);
}





void HandleMessage(MessagingInterface::Message* message)
{
    switch (message->type) {
    case MessagingInterface::kPostLoad:

        //if (LEX::ProjectManager::instance->CreateProject("ActorValueGenerator", nullptr) != LEX::APIResult::Success) { logger::info("AVG has experienced failure"); }

        break;
        // It is now safe to do multithreaded operations, or operations against other plugins.

    case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.

        Component::LinkComponents(LinkFlag::Loaded);

        Component::LinkComponents(LinkFlag::Declaration);

        Component::LinkComponents(LinkFlag::Definition);

        //Component::LinkComponents(LinkFlag::External);


        Initializer::Execute("function_register");
        break;

    case MessagingInterface::kInputLoaded:
        Install();
        break;

    case MessagingInterface::kDataLoaded:
        Component::LinkComponents(LinkFlag::Object);
        break;

    }
}

void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        
        if (auto _ = InvokeOrExit(message->type <= MessagingInterface::kPostPostLoad, [=]() { HandleMessage(message); })) {
            //TODO: MessageBox this instead

            auto name = GetModuleName();
            std::string str = std::format("An unhandled exception has been encountered when interpreting {} message in {}. Please check plugin log for more info.",
                magic_enum::enum_name((decltype(MessagingInterface::kTotal))message->type), name);
            
            MessageBoxA(NULL, str.c_str(), name.c_str(), MB_OK);
            //logger::critical("An unhandled exception has been encountered when interpreting {} message in {}. Please check logs for crash information (a crash",
            //    magic_enum::enum_name((decltype(MessagingInterface::kTotal))message->type),
            //    GetModuleName());
        }
        })) 
    {
        SKSE::stl::report_and_fail("Unable to register message listener.");
    }
}












void LexTesting()
{
    const int&& test1 = 1;

    auto& test2 = test1;

    ProjectManager::instance->InitMain();
    


}




INITIALIZE()
{
    //This gives 1 too many.
    RegisterObjectType<RE::TESForm*>("FORM", (TypeOffset)RE::FormType::Max + ExtraForm::kTotal);    
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



void TestParsing()
{
    using nlohmann::json;
    using nlohmann::json_schema::json_validator;

    static json schema;
    
    try
    {
        schema = json::parse(R"(
{
  "$schema": "https://json-schema.org/draft/2020-12/schema#",
  "title" : "LexiconSKSE",
  "description" : "configuration for Lexicon functors.",
  "type" : "object",
  "properties" : {
    "functions" : {
      "type" : "object",
      "patternProperties": {
        "[_\\w][\\w]*": {
          "properties": {
            "formula": {
              "type" : "string"
            },
            "default":{
              "type": "number" 
            },
            "parameter" :{
              "type" : "string"
            }
          },
          "additionalProperties": false,
          "dependentRequired": {
            "parameter": ["default"]
          },
          "required": ["formula"]
        }
      }
    },
    "properties": {
      "type" : "object",
      "patternProperties": {
        "[_\\w][\\w]*": {
          "oneOf": [
            {
              "properties" : {
                "type" : {
                  "type" : "string",
                  "const" : "float"
                },
                "value":{
                  "type" : "number"
                }
              },
              "additionalProperties": false
            },
            {
              "properties" : {
                "type" : {
                  "type" : "string",
                  "const" : "int"
                },
                "value":{
                  "type" : "integer"
                }
              },
              "additionalProperties": false
            },
            {              
              "properties" : {
                "type" : {
                  "type" : "string",
                  "const" : "bool"
                },
                "value":{
                  "type" : "boolean"
                }
              },
              "additionalProperties": false
            },
            {
              "properties" :{
                "type" : {
                  "type" : "string"
                },
                "value":{
                  "type" : "string"
                }
              },
              "additionalProperties": false
            }  
          ],
          "required": ["value", "type"]
        }  
      }
    },
    "settings" : {
      "properties": {
        "version":{
           "type" : "string",
           "pattern": "\\d+(?:\\.\\d+){0,3}$"
        },
        "parents" : {
          "oneOf" : [
            {
              "type": "array",
              "items": {
                "type" : "string",
                "pattern": ".*\\.es[lmp]$"
               },
               "uniqueItems": true
            },
            {
              "type" : "string",
              "pattern": ".*\\.es[lmp]$"
            }
          ]        
        }
      },
      "additionalProperties": false
    }
  },
  "anyOf": [
      { "required": ["functions"] },
      { "required": ["properties"] }
    ],  
  "additionalProperties": false
}           
            )", nullptr, true, true);
    }
    catch (const std::exception& e) {
        logger::info("Creation of schema failed, here is why: {}", e.what());
        return;
    }

    json_validator validator; // create validator

    try {
        validator.set_root_schema(schema); // insert root-schema
        logger::info("Schema set");

    }
    catch (const std::exception& e) {
        logger::info("Validation of schema failed, here is why: {}", e.what());
        return;
    }

    std::string_view target_string = R"(
            {
	            "settings" : {
		            "parents" :"plugin.esp",
		            "version" : "1"
	            }, 

	            "functions": {
		            "functorName" : {
			            "formula" : " GetActorValue('Health') ",
			            "parameter" : "Form",
			            "default" : 0
		            }

	            },
	            "properties": {
		            "propertyName": {
			            "type" : "float",
			            "value" : "string that evaluates to something."		
		            }
	            }
            }
            )";

    json target;

    try {
        target = json::parse(target_string, nullptr, true, true);
        logger::info("Validation succeeded");
    }
    catch (const std::exception& e) {
        logger::info("Validation failed, here is why: {}", e.what());
        return;
    }

    std::stringstream stream;
    stream << target;
    logger::info("test:\n{}", stream.str());
    try {
        validator.validate(target); // validate the document - uses the default throwing error-handler
        logger::info("Validation succeeded");
    }
    catch (const std::exception& e) {
        logger::info("Validation failed, here is why: {}", e.what());
    }
}

SKSEPluginLoad(const LoadInterface* skse) {
    //SettingManager::SetSettingPath("");
    
    logger::InitializeLogging();
    logger::info("laoding");
    //SETTING_PATH;
    
#ifdef _DEBUG

    

    if (GetKeyState(VK_RCONTROL) & 0x800) {
        constexpr auto text1 = L"Request for debugger detected. If you wish to attach one and press Ok, do so now if not please press Cancel.";
        constexpr auto text2 = L"Debugger still not detected. If you wish to continue without one please press Cancel.";
        constexpr auto caption = L"Debugger Required";

        int input = 0;

        do
        {
            input = MessageBox(NULL, !input ? text1 : text2, caption, MB_OKCANCEL);
        } while (!IsDebuggerPresent() && input != IDCANCEL);
    }
#endif

    TestParsing();

    TestFunction();
    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);
    Init(skse, false);

    //TODO: Move this, the version check is more relevant to lexicon in general than this one specfically
    LEX::InterfaceManager::AddVersionCheck([](uintptr_t server, uintptr_t client) -> LEX::Update
        {
//Move and preserve
#define SEM_VER(mc_major, mc_minor, mc_change, mc_fixes) \
            mc_major##ULL   <<  0 | \
            mc_minor##ULL   <<  8 | \
            mc_change##ULL  << 16 | \
            mc_fixes##ULL   << 24

            constexpr auto not_allowed = SEM_VER(1, 0, 0, 0);

            if (client <= not_allowed) {
                return LEX::Update::Library;
            }

            return LEX::Update::Match;
        });


    Initializer::Execute("main_init");
    Initializer::Execute();
    InitializeMessaging();

    ProjectManager::instance->InitMain();
    

    log::info("{} has finished loading.", plugin->GetName());


    return true;
}


namespace LEX
{

    enum struct DispatchType
    {
        Static,
        Method,
        Event,
    };

    class PapyrusDispatcher : public RE::BSScript::IFunctionArguments
    {
    public:
        PapyrusDispatcher(RE::TESForm* self, std::span<Variable> args) : _vars{ args }
        {
            /*
            //copy A please
            if (tar) {
                RE::BSScript::Variable buffer;
                if (LoadVariable(*tar, buffer) == true) {
                    _target = buffer.GetObject();
                }
            }
            //*/

            RE::BSScript::Variable buffer;
            buffer.Pack(self);
            _target = buffer.GetObject();
        }

        PapyrusDispatcher(std::span<Variable> args) : _vars{ args }
        {
        }

        ~PapyrusDispatcher() override = default;


        bool LoadVariable(Variable& var, RE::BSScript::Variable& arg) const
        {
            switch (var.index())
            {
            case VariableEnum::Number: {
                auto& number = var.AsNumber();



                //TODO: This needs to be repurposed to be able to handle certain things such as unsigned bools and different integer types
                switch (number.GetOffset())
                {
                case Number::Settings::CreateFromType<int>().GetOffset():
                case Number::Settings::CreateFromType<int64_t>().GetOffset():
                    arg.SetSInt(number);
                    break;
                case Number::Settings::CreateFromType<double>().GetOffset():
                case Number::Settings::CreateFromType<float>().GetOffset():
                    arg.SetFloat(number);
                    break;

                case Number::Settings::CreateFromType<bool>().GetOffset():
                    arg.SetBool(number);
                    break;
                default:
                    logger::error("something 1");
                    return false;
                }
                
                break;
            }

            case VariableEnum::String: {
                arg.SetString(var.AsString());
                break;
            }

            case VariableEnum::Object: {
                //Needs form, array, alias, active effect
                Object& object = var.AsObject();

                if (auto policy = object.policy)
                {
                    policy->GetCategoryIndex();
                    switch (Hash(policy->GetCategoryName()))
                    {
                        //case "AME"_h:
                        //case "ALIAS"_h:

                    case "FORM"_h: {
                        //Later, this should use the something get the offset straight from the object, possibly pulling from a stored value
                        // if it's considered a null value
                        //arg.Pack(object.get<RE::TESForm*>());
                        auto form = object.get<RE::TESForm*>();
                        

                        RE::BSScript::PackHandle(&arg, form, static_cast<RE::VMTypeID>(form ? form->formType.get() : RE::FormType::None));
                        break;
                    }

                    case "ARRAY"_h: {

                        //break;
                    }
                    default:
                        logger::error("something 2");
                        return false;
                    }
                }
                else {
                    logger::error("something 3");
                    return false;
                }

                break;
            }

            default:
                logger::error("something 4");
                return false;

            }

            return true;

        }

        bool operator()(RE::BSScrapArray<RE::BSScript::Variable>& out) const override
        {
            out.reserve(_vars.size());

            for (auto& var : _vars)
            {
                RE::BSScript::Variable arg;

                if (LoadVariable(var, arg) == false)
                    return false;
                
                out.push_back(arg);
            }

            return true;
        }

        //There will be 2 versions of this function, one with an out, with maybe a few different ways to assign it, like a function or a variable
        bool CallImpl(std::string_view class_name, std::string_view func_name, DispatchType type, Variable* out)
        {
            //RE::BSScript::IStackCallbackFunctor;
            auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            
            if (!vm)
                return false;

            if (vm->IsCompletelyFrozen() == true)
                return false;

            bool result;

            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;

            switch (type)
            {
            case DispatchType::Static:
                result = vm->DispatchStaticCall(class_name, func_name, this, callback);
                break;

            case DispatchType::Method:
                result = vm->DispatchMethodCall(_target, func_name, this, callback);
                break;

            case DispatchType::Event: {
                auto vm = RE::SkyrimVM::GetSingleton();
                RE::BSFixedString name = func_name;


                vm->SendAndRelayEvent(_target->GetHandle(), &name, this, nullptr);
                result = true;
                break;
            }

            default:
                return false;
            }

            return result;
        }

        
        bool StaticCall(std::string_view class_name, std::string_view func_name, Variable* out)
        {
            return CallImpl(class_name, func_name, DispatchType::Static, out);
        }

        bool MethodCall(std::string_view func_name, Variable* out)
        {
            return CallImpl(""sv, func_name, DispatchType::Method, out);
        }

        bool SendEvent(std::string_view func_name)
        {
            return CallImpl(""sv, func_name, DispatchType::Event, nullptr);
        }

    private:
        RE::BSTSmartPointer<RE::BSScript::Object> _target;
        std::span<Variable> _vars;

       
    };


    //TODO: MOve this elsewhere, these need a proper home in the function registers.
    bool CallPapyrusFunction(StaticTargetTag, std::string_view class_name, std::string_view func_name, variadic<Variable> args)
    {
        PapyrusDispatcher dispatcher{ args };

        return dispatcher.StaticCall(class_name, func_name, nullptr);
    }

    //This works but fails to work probably with any loaded arguments
    bool CallPapyrusMethod(RE::TESForm* self, std::string_view func_name, variadic<Variable> args)
    {
        PapyrusDispatcher dispatcher{ self, args };

        return dispatcher.MethodCall(func_name, nullptr);
    }

    bool SendPapyrusEvent(RE::TESForm* self, std::string_view func_name, variadic<Variable> args)
    {
        PapyrusDispatcher dispatcher{ self, args };

        return dispatcher.SendEvent(func_name);
    }


    //TODO: Initialize here happens multiple times, and is mainly stored in the non

    inline void __init_func_38(); inline ::Initializer __init_var_38 = { __init_func_38 , "function_register" }; inline void __init_func_38()
    {
        RegisterDump dump;
        logger::debug("Papyrus Interface Functions:");
        dump = ProcedureHandler::instance->RegisterFunction(CallPapyrusMethod, "Shared::GameObjects::CallPapyrusMethod");
        dump = ProcedureHandler::instance->RegisterFunction(CallPapyrusFunction, "Shared::GameObjects::CallPapyrusFunction");
        dump = ProcedureHandler::instance->RegisterFunction(SendPapyrusEvent, "Shared::GameObjects::SendPapyrusEvent");
    }



    //The goal is to attempt to make this some other game object. The general idea of game object is I can use it to represent the core scripted objects.
    //Note though, not important right now due to having no use for it
    //Scratch that, I need this now. The idea should be to ensure that the type targeted is a game object
    struct GameObject
    {
        //Additionally, I'd like to make something called a proxy object, which is basically just a carrier of a specific kind of object, but layers
        // modifiers over it.

        GameObject(Object& obj) : object{ obj } {}

        Object object;
        


    };


    void TestingCompile()
    {
        LEX_VERSION;
        Script* script = nullptr;
        SyntaxRecord record;
        script->AppendContent(record);
    }
    




}

