#include "GameObjectStuff.h"
#include "Lexicon/Engine/TempConstruct.h"
#include "Lexicon/Interfaces/InterfaceManager.h"

#include "FunctionRegister.h"
#include "Lexicon/Engine/SettingManager.h"


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

    RGL_LOG(info, "{}{}", indent, log);

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

void InitializeMessaging() {
    if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
        switch (message->type) {
        case MessagingInterface::kPostLoad:

            //if (LEX::ProjectManager::instance->CreateProject("ActorValueGenerator", nullptr) != LEX::APIResult::Success) { logger::info("AVG has experienced failure"); }

            break;
            // It is now safe to do multithreaded operations, or operations against other plugins.

        case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.

            Component::Link(LinkFlag::Loaded);

            Component::Link(LinkFlag::Declaration);

            Component::Link(LinkFlag::Definition);

            temp_NativeFormulaRegister();

            break;

        case MessagingInterface::kDataLoaded:
            logger::info("s1");
            {
                //auto something = Formula<RE::PlayerCharacter*>::Run("Shared::GameObjects::GetPlayer()");
                //Formula<void>::Run("Shared::GameObjects::GetPlayer().Shared::GameObjects::DoNothing()");

                Component::Link(LinkFlag::External);

                //logger::info("Project {}", LEX::Formula<float>::Run("GetPlayer().ProjectTest()", "ActorValueGenerator::Commons"));
                
            }

            logger::info("s2");
            //break;
            [[fallthrough]];
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



void LexTesting()
{
    ProjectManager::instance->InitMain();
    

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
    //SettingManager::SetSettingPath("");
    
    logger::InitializeLogging();
    //SETTING_PATH;
    
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
    //TestParse();

    log::info("{} has finished loading.", plugin->GetName());


    return true;
}
