#include "GameObjectStuff.h"
#include "Lexicon/Engine/TempConstruct.h"

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


void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        switch (message->type) {
        case MessagingInterface::kPostLoad:

            break;
            // It is now safe to do multithreaded operations, or operations against other plugins.

        case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.

            break;

        case MessagingInterface::kDataLoaded:

        case MessagingInterface::kSaveGame:
        {
            //*
            RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
            
            Variable result = function->Call(MakeObject(player));
            
            std::string number = result.AsNumber().ToString();
            RE::DebugNotification(number.c_str());
            report::info("form id of {} is {} as a double", player->GetDisplayFullName(), number);
            //*/
        }
            break;
        }
        })) {
        SKSE::stl::report_and_fail("Unable to register message listener.");
    }
}



double GetFormIdAsDouble(RE::TESForm* a_this)
{
    return a_this ? a_this->GetFormID() : 0.0;
}

void LexTesting()
{
    logger::debug("a");
    ProjectManager::instance->InitMain();
    logger::debug("b");
    Component::Link(LinkFlag::Declaration);
    logger::debug("c");
    Component::Link(LinkFlag::Definition);
    logger::debug("d");
    Component::Link(LinkFlag::External);
    logger::debug("e");
    Script* script = ProjectManager::instance->GetShared()->FindScript("GameObjects");
    logger::debug("f");
    //return;
    //ProjectManager::instance->GetFunctionFromPath("Shared::Commons::size");
    
        auto funcs = script->FindFunctions("GetFormIdAsDouble");
        logger::debug("g");
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
            else logger::debug("hell1");
        }
        else logger::debug("hell2");
}







SKSEPluginLoad(const LoadInterface* skse) {
    
    logger::InitializeLogging();
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
    TestFunction();
    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);
    Init(skse);
    Initializer::Execute();

    InitializeMessaging();

    LexTesting();

    log::info("{} has finished loading.", plugin->GetName());


    return true;
}
