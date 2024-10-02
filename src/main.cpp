#include "GameObjectStuff.h"
#include "Lexicon/Engine/TempConstruct.h"
#include "Lexicon/Interfaces/InterfaceManager.h"

#include "FunctionRegister.h"

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

//using namespace SOS;
//using namespace RGL;

using namespace LEX;


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
    const auto level = spdlog::level::trace;
#else
    //Use right alt for just debug logging, control to allow debugger to attach.
    const auto level = GetKeyState(VK_RCONTROL) & 0x800 || GetKeyState(VK_RMENU) & 0x800 ?
        spdlog::level::debug : spdlog::level::info;
#endif


    log->set_level(level);
    log->flush_on(level);

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


static ConcreteFunction* function = nullptr;
static ConcreteFunction* actorValueFunc = nullptr;

void LogDis(std::string_view name, float value)
{
    logger::info("The current health of {} is {}", name, value);
}

void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        switch (message->type) {
        case MessagingInterface::kPostLoad:

            break;
            // It is now safe to do multithreaded operations, or operations against other plugins.

        case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.

            break;

        case MessagingInterface::kDataLoaded:
            Component::Link(LinkFlag::External);

            //logger::info("a");
            //break;
        case MessagingInterface::kSaveGame:
        {
            Interface* intf = nullptr;
            //RequestInterface_Impl(intf, "something", 1);

            //*
            logger::info("a");
            //return;
            RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
            logger::info("b");
            Object test = MakeObject(player);
            logger::info("c");
            Object test2 = test;
            logger::info("d");
            //float number = Formula<float>::Run("Shared::GameObjects::GetPlayer().Shared::GameObjects::GetActorValue('Health')");
            float number = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
            //float number = 65;
            //float number = 100;
            IdentityManager;
            auto something = Formula<RE::PlayerCharacter*>::Run("Shared::GameObjects::GetPlayer()");
            Formula<void>::Run("Shared::GameObjects::GetPlayer().Shared::GameObjects::DoNothing()");
            number = Formula<float>::Run("GetPlayer().GetActorValue('Health')");

            logger::info("e");
            //Variable result = actorValueFunc->Call(player, "Health");
            
            //std::string number = result.AsNumber().string();
            
            LogDis("player->GetDisplayFullName()", number);

            using TTT = float(RE::Actor::*)();
            
            number = 0;
            
            report::info("resetting...");

            auto form = Formula<float(RE::Actor::*)()>::Create("GetActorValue('Health')");
            
            number = form(player);

            report::info("player->GetDisplayFullName() {}", number);
            if constexpr (0)
            {

                constexpr auto text1 = L"Request for debugger detected. If you wish to attach one and press Ok, do so now if not please press Cancel.";
                constexpr auto text2 = L"Debugger still not detected. If you wish to continue without one please press Cancel.";
                constexpr auto caption = L"Debugger Required";

                int input = 0;

                do
                {
                    input = MessageBox(NULL, !input ? text1 : text2, caption, MB_OKCANCEL);
                } while (!IsDebuggerPresent() && input != IDCANCEL);
            }

            unsigned int levelTest = Formula<unsigned int>::Run("(PlayerToActor() as Actor).GetLevel()");


            report::info("player level is {}", levelTest);
            //*/
        }
            break;
        }
        })) {
        SKSE::stl::report_and_fail("Unable to register message listener.");
    }
}

void TestEm()
{

}

static RE::PlayerCharacter* GetPlayer_(StaticTargetTag)
{
    return RE::PlayerCharacter::GetSingleton();
}


double GetFormIdAsDouble(RE::TESForm* a_this)
{
    return a_this ? a_this->GetFormID() : 0.0;
}

float GetActorValue_backend(RE::Actor* a_this, String av_name)
{
    RE::ActorValue av = RE::ActorValueList::GetSingleton()->LookupActorValueByName(av_name);
    logger::info("testing {}, av gotten {}", av_name.view(), magic_enum::enum_name(av));

    return a_this && av != RE::ActorValue::kNone ? a_this->AsActorValueOwner()->GetActorValue(av) : 0.0f;
}

void LexTesting()
{

    Variable test;

    static_cast<std::string_view>(test);

    ProjectManager::instance->InitMain();
    
    Component::Link(LinkFlag::Loaded);

    Component::Link(LinkFlag::Declaration);
    
    Component::Link(LinkFlag::Definition);
    
    //Component::Link(LinkFlag::External);
    
    Script* script = ProjectManager::instance->GetShared()->FindScript("GameObjects");
    
    //return;
    //ProjectManager::instance->GetFunctionFromPath("Shared::Commons::size");
    
        auto funcs = script->FindFunctions("GetFormIdAsDouble");
        
        if (funcs.size() != 0)
        {

            function = dynamic_cast<ConcreteFunction*>(funcs[0]->Get());

            if (function)
            {
                /*
                static_assert(LEX::detail::function_has_var_type<double>, "false");
                static_assert(LEX::detail::function_has_var_type<RE::TESForm*>, "false");
                //static_assert(LEX::detail::call_class_has_var_type_Store<RE::TESForm>, "false");
                static_assert(LEX::detail::call_class_has_var_type_Value<RE::TESForm*>, "false");
                constexpr auto testing = !std::is_base_of_v<LEX::detail::not_implemented, VariableType<RE::TESForm*>>;

                static_assert(testing);
                static_assert(testing  && requires(const RE::TESForm* t)
                {
                    { LEX::detail::ObtainVariableType<RE::TESForm*>()(t) } -> pointer_derived_from<AbstractTypePolicy*>;
                }, "false");

                LEX::detail::ObtainVariableType<RE::TESForm>();

                RE::TESForm* testForm = nullptr;

                VariableType<RE::TESForm*>{}(testForm);

                GetVariableType<RE::TESForm*>();
                //*/
                if (ProcedureHandler::instance->RegisterFunction(GetFormIdAsDouble, function) == false) {
                    report::critical("failure");
                }
                else
                {
                    report::info("success");
                }
            }
        }


        funcs = script->FindFunctions("GetActorValue");
        
        if (funcs.size() != 0)
        {

            actorValueFunc = dynamic_cast<ConcreteFunction*>(funcs[0]->Get());

            if (actorValueFunc)
            {
                /*
                static_assert(LEX::detail::function_has_var_type<double>, "false");
                static_assert(LEX::detail::function_has_var_type<RE::TESForm*>, "false");
                //static_assert(LEX::detail::call_class_has_var_type_Store<RE::TESForm>, "false");
                static_assert(LEX::detail::call_class_has_var_type_Value<RE::TESForm*>, "false");
                constexpr auto testing = !std::is_base_of_v<LEX::detail::not_implemented, VariableType<RE::TESForm*>>;

                static_assert(testing);
                static_assert(testing  && requires(const RE::TESForm* t)
                {
                    { LEX::detail::ObtainVariableType<RE::TESForm*>()(t) } -> pointer_derived_from<AbstractTypePolicy*>;
                }, "false");

                LEX::detail::ObtainVariableType<RE::TESForm>();

                RE::TESForm* testForm = nullptr;

                VariableType<RE::TESForm*>{}(testForm);

                GetVariableType<RE::TESForm*>();
                //*/
                if (ProcedureHandler::instance->RegisterFunction(GetActorValue_backend, actorValueFunc) == false) {
                    report::critical("failure");
                }
                else
                {
                    report::info("success");
                }
            }
        }


        
        if (ProcedureHandler::instance->RegisterFunction(GetPlayer_, "Shared::GameObjects::GetPlayer") == false)
        {
            logger::info("Function couldn't be set");
        }

        temp_NativeFormulaRegister();
}



INITIALIZE()
{

    //Put this sort of thing in a unique ptr please.
    DefaultClient::SetInstance(new DefaultClientEx);

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




SKSEPluginLoad(const LoadInterface* skse) {
    
    logger::InitializeLogging();
//#ifdef _DEBUG

    

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
//#endif
    TestFunction();
    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);
    Init(skse);
    logger::info("___A");
    Initializer::Execute();
    logger::info("___B");
    InitializeMessaging();
    logger::info("___C");
    LexTesting();
    log::info("{} has finished loading.", plugin->GetName());


    return true;
}
