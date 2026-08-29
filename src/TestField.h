#pragma once


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

#include "ConditionAttribute.h"
namespace LEX
{
    template <size_t I, size_t... Vs>
    struct number_pack : public number_pack<I - 1, Vs..., I - 1>
    {

    };

    template <size_t... Vs>
    struct number_pack<0, Vs...> 
    {
        auto operator()(size_t value, auto func)
        {
            decltype(&decltype(func)::template operator()<0>) send = nullptr;

            ((value == Vs && !send ? (send = &decltype(func)::template operator()<Vs>) : send), ...);

            if (!send) {
                func.template operator()<0>();
            }

            return (func.*send)();
        }
    };


    /*
    BinarySearch revision.
    What I think I'll do is make a type that has no vtable or rtti actively used where it will take a number, and a parameter pack
     it derives from itself minus 1, all of its parameter pack, and another void. If the number is 0 it will be truly zero.
     From this parameter pack I will create a number sequence
     actually nevermind, I'll fill it with numbers.
     


    //*/

    void ReadConditionFunctions()
    {

    }
#ifdef USE_TEST_FIELD
    //Instead of this, I'll give it a literal type info, and I'll turn the result into a number, and convert it to the 
    // expected type
    enum struct ReturnType
    {
        Float,
        Integer,
        Boolean,
        Enum,
        Def,
        NotDoing
    };


    using FunctionID = RE::FUNCTION_DATA::FunctionID;



    ReturnType GetObscriptReturnType(RE::SCRIPT_FUNCTION* function)
    {
        using FunctionID = RE::FUNCTION_DATA::FunctionID;

        FunctionID id = (FunctionID)(std::to_underlying(function->output) - 4096);

        switch (id)
        {
            //GetVATSMode- unknown
        case FunctionID::kMenuMode:
        case FunctionID::kGetScriptVariable:
        case FunctionID::kCanHaveFlames:
        case FunctionID::kGetAnimAction:
        case FunctionID::kGetPersuasionNumber:
        case FunctionID::kGetClassDefaultMatch:
        case FunctionID::kGetTotalPersuasionNumber:
        case FunctionID::kGetNoRumors:
        case FunctionID::kGetHitLocation:
        case FunctionID::kGetCauseofDeath://This might be used, but is simply never set.
        case FunctionID::kGetVATSValue://Used but highly complicated, would likely be floating in value
        //case FunctionID::kMenuMode:
            return ReturnType::NotDoing;



        case FunctionID::kGetKnockedState:
        case FunctionID::kGetWeaponAnimType:
        case FunctionID::kGetCurrentAIProcedure:
        case FunctionID::kGetCurrentAIPackage:
        case FunctionID::kGetOpenState:
        case FunctionID::kGetDayOfWeek:
        case FunctionID::kGetPlayerAction:
        case FunctionID::kGetCombatState:
        case FunctionID::kGetLastPlayerAction:
        //case FunctionID::kGetKnockedState:


            return ReturnType::Enum;

        case FunctionID::kGetItemCount:
        case FunctionID::kGetSecondsPassed:
        case FunctionID::kGetGold:
        case FunctionID::kGetStage:
        case FunctionID::kGetFactionRankDifference:
        case FunctionID::kGetLockLevel:
        case FunctionID::kGetFactionRank:
        case FunctionID::kGetLevel:
        case FunctionID::kGetDeadCount:
        case FunctionID::kGetTrespassWarningLevel:
        case FunctionID::kGetAmountSoldStolen:
        case FunctionID::kGetFriendHit:
        case FunctionID::kGetStolenItemValueNoCrime:
        case FunctionID::kGetStolenItemValue:
        case FunctionID::kGetCrimeGoldViolent:
        case FunctionID::kGetCrimeGoldNonviolent:
        case FunctionID::kGetRelationshipRank:
        //case FunctionID::kGetItemCount:
            return ReturnType::Integer;

        case FunctionID::kGetDisease:
        case FunctionID::kGetDetected:
        case FunctionID::kGetDead:
        case FunctionID::kGetTalkedToPC:
        case FunctionID::kGetSleeping:
        case FunctionID::kGetQuestRunning:
        case FunctionID::kGetStageDone:
        case FunctionID::kGetAlarmed:
        case FunctionID::kGetAttacked:
        case FunctionID::kGetShouldAttack:
        case FunctionID::kGetActorAggroRadiusViolated:
        case FunctionID::kGetCrime:
        case FunctionID::kGetSitting:
        case FunctionID::kGetTalkedToPCParam:
        case FunctionID::kGetEquipped:
        case FunctionID::kGetDestroyed:
        case FunctionID::kGetDefaultOpen:
        case FunctionID::kGetVampireFeed:
        case FunctionID::kGetCannibal:
        case FunctionID::kGetUnconscious:
        case FunctionID::kGetRestrained:
        case FunctionID::kGetLocationCleared:
        case FunctionID::kGetOffersServicesNow:
        case FunctionID::kGetPairedAnimation:
        case FunctionID::kGetIdleDoneOnce:
        case FunctionID::kGetWithinPackageLocation:
        //case FunctionID::kGetDisease:
        //case FunctionID::kGetDisease:
        //case FunctionID::kGetDisease:
        case FunctionID::kGetDisabled:
        case FunctionID::kGetLineOfSight:
        case FunctionID::kGetWantBlocking:
            return ReturnType::Boolean;

        case FunctionID::kGetPCMiscStat:
            return ReturnType::Float;

        default:
            if constexpr (1)
            {
                std::string_view name = function->functionName;

                if (name.starts_with("Is") ||
                    name.starts_with("GetIs") ||
                    name.starts_with("GetIn") ||
                    name.starts_with("GetPC") ||
                    name.starts_with("GetIgnore") ||
                    name.starts_with("Can") ||
                    name.contains("Same") ||
                    name.contains("Has")
                    )
                {
                    return ReturnType::Boolean;
                }


                return ReturnType::Def;
            };
            break;

        }
    }




    void ReadConditionFunctions()
    {
        using FunctionID = RE::FUNCTION_DATA::FunctionID;
        RE::SCRIPT_FUNCTION* it = RE::SCRIPT_FUNCTION::GetFirstScriptCommand();

        constexpr auto zero = (FunctionID)0;

        logger::debug("STARTING NOW\n");


        for (FunctionID i = zero; i < FunctionID::kTotal; i++)
        {

            RE::SCRIPT_FUNCTION& func = it[std::to_underlying(i)];

            bool has_condition = func.conditionFunction;

            if (!has_condition) {
                continue;
            }

            if (GetObscriptReturnType(&func) != ReturnType::Def) {
                continue;
            }

            if (!!i)
                logger::debug("=======================================================");

            std::string_view name = func.functionName;
            std::string_view help = func.helpString;

            String test1;
            RE::BSFixedString test = test1;
            bool cell_inval = func.invalidatesCellList;
            std::string_view smol_name = func.shortName;
            auto output_unk = std::to_underlying(func.output);
            bool editor = func.editorFilter;
            logger::debug("|  {}({}): {}", name, smol_name, help);
            logger::debug("|  |  output: {}", output_unk);
            logger::debug("|  |  editor: {}", editor);
            logger::debug("|  |  cond exists: {}", has_condition);
            logger::debug("|  |  invalidates cell list: {}", cell_inval);

            int param_count = func.numParams;

            logger::debug("|  |  Params({}):-----------------------------------------", param_count);


            RE::SCRIPT_PARAMETER* params = func.params;
            for (int i = 0; i < param_count; i++) {
                if (i)
                    logger::debug("----------------------------------------------------------");

                RE::SCRIPT_PARAMETER& param = params[i];

                //if optional, make a second that calls the other.
                bool optional = param.optional;
                std::string_view name = param.paramName;
                std::string_view type = magic_enum::enum_name(param.paramType.get());
                logger::debug("|  |  |  Name: {}", name);
                logger::debug("|  |  |  Type: {}", type);
                logger::debug("|  |  |  Opt: {}", optional);



            }
        }

        //func = script->conditionFunction;

    }



    constexpr RE::SCRIPT_PARAM_TYPE EMPTY_PARAM = (RE::SCRIPT_PARAM_TYPE)-1;
    constexpr RE::SCRIPT_PARAM_TYPE PARAM_TOTAL = (RE::SCRIPT_PARAM_TYPE)((int)RE::SCRIPT_PARAM_TYPE::kRegion + 1);


    struct nothing {};

    //If nothing is returned that parameter is unused.
    
#pragma region Obscript Helper

    template <RE::SCRIPT_PARAM_TYPE ParamType>
    struct ObsScriptHelper
    {
        //TypeInfo* GetType(){return nullptr;}


        auto operator()(Variable* arg)
        {
            //returns nothing.
            return Void{};
        }
    };

    template<typename T>
    struct HelperType
    {
        TypeInfo* GetType() { return GetVariableType<T>(); }
    };
    

    template <StringLiteral Type>
    using IForm = Util::Parameter<Type, RE::TESForm*>;



    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kChar> : public HelperType<std::string_view>
    {
        


        RE::BSFixedString operator()(Variable* arg)
        {
            std::string_view value = arg->AsString();

            return value;
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kInt>
    {
        int operator()(Variable* arg)
        {
            return arg->AsNumber();
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kFloat>
    {
        float operator()(Variable* arg)
        {
            return arg->AsNumber();
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kObjectRef>
    {
        RE::TESObjectREFR* operator()(Variable* arg)
        {
            return Unvariable<RE::TESObjectREFR*>{}(arg);
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kActorValue> : public HelperType<std::string_view>
    {
        RE::ActorValue operator()(Variable* arg)
        {
            return RE::ActorValueList::LookupActorValueByName(arg->AsString().c_str());
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kActor>
    {
        RE::Actor* operator()(Variable* arg)
        {
            return Unvariable<RE::Actor*>{}(arg);
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kSpellItem>
    {
        RE::SpellItem* operator()(Variable* arg)
        {
            return Unvariable<RE::SpellItem*>{}(arg);
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kAxis> : public HelperType<std::string_view>
    {
        char operator()(Variable* arg)
        {
            std::string_view value = arg->AsString();

            switch (Hash(value))
            {
            case "X"_h:
            case "Y"_h:
            case "Z"_h:
                return value[0];


            case "x"_h:
            case "y"_h:
            case "z"_h:
                return std::toupper(value[0]);

            default:
                return '\0';

            }
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kSex> : public HelperType<std::string_view>
    {
        RE::SEX operator()(Variable* arg)
        {
            std::string_view value = arg->AsString();

            switch (Hash<HashFlags::Insensitive>(value))
            {
            case "male"_ih:
                return RE::SEX::kMale;
            
            case "female"_ih:
                return RE::SEX::kFemale;
                
            default:
                return RE::SEX::kNone;

            }
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kCell>
    {
        RE::TESObjectCELL* operator()(Variable* arg)
        {
            return Unvariable<RE::TESObjectCELL*>{}(arg);
        }
    };



    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kMagicItem>
    {
        RE::MagicItem* operator()(Variable* arg)
        {
            return Unvariable<RE::MagicItem*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kQuest>
    {
        RE::TESQuest* operator()(Variable* arg)
        {
            return Unvariable<RE::TESQuest*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kRace>
    {
        RE::TESRace* operator()(Variable* arg)
        {
            return Unvariable<RE::TESRace*>{}(arg);
        }
    };
    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kClass>
    {
        RE::TESClass* operator()(Variable* arg)
        {
            return Unvariable<RE::TESClass*>{}(arg);
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kFaction>
    {
        RE::TESFaction* operator()(Variable* arg)
        {
            return Unvariable<RE::TESFaction*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kGlobal>
    {
        RE::TESGlobal* operator()(Variable* arg)
        {
            return Unvariable<RE::TESGlobal*>{}(arg);
        }
    };




    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kFurnitureOrFormList> : public HelperType<IForm<"Shared::Interfaces::IFurnitureOrList">>
    {
        RE::TESForm* operator()(Variable* arg)
        {
            return Unvariable<RE::TESForm*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kObject>
    {
        RE::TESBoundObject* operator()(Variable* arg)
        {
            return Unvariable<RE::TESBoundObject*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kStage>
    {
        uint16_t operator()(Variable* arg)
        {
            return arg->AsNumber();
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kActorBase>
    {
        RE::TESNPC* operator()(Variable* arg)
        {
            return Unvariable<RE::TESNPC*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kWorldOrList> : public HelperType<IForm<"Shared::Interfaces::IWorldOrList">>
    {
        RE::TESForm* operator()(Variable* arg)
        {
            return Unvariable<RE::TESForm*>{}(arg);
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kCrimeType>
    {
        CRIME_TYPE operator()(Variable* arg)
        {
            return Unvariable<CRIME_TYPE>{}(arg);
        }
    };

    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kPackage>
    {
        RE::TESPackage* operator()(Variable* arg)
        {
            return Unvariable<RE::TESPackage*>{}(arg);
        }
    };


    template <>
    struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kMagicEffect>
    {
        RE::EffectSetting* operator()(Variable* arg)
        {
            return Unvariable<RE::EffectSetting*>{}(arg);
        }
    };


template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kCombatStyle>
{
    RE::TESCombatStyle* operator()(Variable* arg)
    {
        return Unvariable<RE::TESCombatStyle*>{}(arg);
    }
};


template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kRefType>
{
    RE::BGSLocationRefType* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSLocationRefType*>{}(arg);
    }
};

template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kBGSScene>
{
    RE::BGSScene* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSScene*>{}(arg);
    }
};


template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kWeather>
{
    RE::TESWeather* operator()(Variable* arg)
    {
        return Unvariable<RE::TESWeather*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kPerk>
{
    RE::BGSPerk* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSPerk*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kVoiceType>
{
    RE::BGSVoiceType* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSVoiceType*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kEncounterZone>
{
    RE::BGSEncounterZone* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSEncounterZone*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kIdleForm>
{
    RE::TESIdleForm* operator()(Variable* arg)
    {
        return Unvariable<RE::TESIdleForm*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kMessage>
{
    RE::BGSMessage* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSMessage*>{}(arg);
    }
};



//template <>
//struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kEquipType>
//{
//    RE::BGSEquipType* operator()(Variable* arg)
//    {
//        return Unvariable<RE::BGSEquipType*>{}(arg);
//    }
//};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kKeyword>
{
    RE::BGSKeyword* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSKeyword*>{}(arg);
    }
};






template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kLocation>
{
    RE::BGSLocation* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSLocation*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kForm>
{
    RE::TESForm* operator()(Variable* arg)
    {
        return Unvariable<RE::TESForm*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kShout>
{
    RE::TESShout* operator()(Variable* arg)
    {
        return Unvariable<RE::TESShout*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kWordOfPower>
{
    RE::TESWordOfPower* operator()(Variable* arg)
    {
        return Unvariable<RE::TESWordOfPower*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kAssociationType>
{
    RE::BGSAssociationType* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSAssociationType*>{}(arg);
    }
};



template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kRegion>
{
    RE::TESRegion* operator()(Variable* arg)
    {
        return Unvariable<RE::TESRegion*>{}(arg);
    }
};

template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kFormList>
{
    RE::BGSListForm* operator()(Variable* arg)
    {
        return Unvariable<RE::BGSListForm*>{}(arg);
    }
};

template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kFormType>
{
    RE::FormType operator()(Variable* arg)
    {
        return Unvariable<RE::FormType>{}(arg);
    }
};





template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kOwner> : public HelperType<IForm<"Shared::Interfaces::IOwner">>
{
    RE::TESForm* operator()(Variable* arg)
    {
        return Unvariable<RE::TESForm*>{}(arg);
    }
};














template <>
struct ObsScriptHelper<RE::SCRIPT_PARAM_TYPE::kMiscStat> : public HelperType<std::string_view>
{
    uint32_t operator()(Variable* arg)
    {
        std::string_view value = arg->AsString();

        return RE::BSCRC32<std::string_view>{}(value);
    }
};


#pragma endregion    


    void TestUnvarEnum()
    {
        SKSE::PluginVersionData;
        RE::TESWeather;
        
        Unvariable<RE::ActorValue>{};
    }



    bool IsParamTypeAllowed(RE::SCRIPT_PARAM_TYPE param_type)
    {
        bool result = false;
        BinarySearch<(int)PARAM_TOTAL>((int)param_type, [&]<size_t I> [[msvc::force_inline]] {

                using ParamType = std::invoke_result_t<ObsScriptHelper<(RE::SCRIPT_PARAM_TYPE)I>, Variable*>;

                result = !std::is_same_v<Void, ParamType>;
        });

        return result;

    }


    TypeInfo* GetTypeFromParam(RE::SCRIPT_PARAM_TYPE param_type)
    {
        TypeInfo* result = nullptr;

        BinarySearch<(int)PARAM_TOTAL>((int)param_type, [&]<size_t I> [[msvc::force_inline]] {
            
            if constexpr (requires(ObsScriptHelper<(RE::SCRIPT_PARAM_TYPE)I> functor) { { functor.GetType() } -> std::same_as<TypeInfo*>; })
            {
                result = ObsScriptHelper<(RE::SCRIPT_PARAM_TYPE)I>{}.GetType();
            }
            else {
                using ParamType = std::invoke_result_t<ObsScriptHelper<(RE::SCRIPT_PARAM_TYPE)I>, Variable*>;

                if constexpr (!std::is_same_v<Void, ParamType>) {
                    result = GetVariableType<ParamType>();
                }
            }
        });

        return result;

    }


    template<RE::SCRIPT_PARAM_TYPE ParamType>
    auto GetArgFromParam(size_t i, std::span<Variable*>& args)
    {
        using Type = std::invoke_result_t<ObsScriptHelper<ParamType>, Variable*>;

        if constexpr (!std::is_same_v<Void, Type>) {
            return ObsScriptHelper<ParamType>{}(args[i]);
        }
        else {
            return Void{};
        }
        
    }

    void TestRefr()
    {
        Unvariable<RE::TESObjectREFR*>{}(nullptr);
    }
    


    template<typename RE::SCRIPT_PARAM_TYPE P1 = EMPTY_PARAM, typename RE::SCRIPT_PARAM_TYPE P2 = EMPTY_PARAM>
    struct ObScriptDispatcher : public Dispatcher
    {
        ObScriptDispatcher(RE::SCRIPT_FUNCTION* func) : function{ func } {}

        template <typename T>
        static void* Send(T& value)
        {
            if constexpr (std::is_same_v<T, Void>) {
                return nullptr;
            }
            if constexpr (std::is_same_v<T, RE::BSFixedString>) {
                return std::addressof(value);
            }
            else {
                union
                {
                    T in;
                    void* out;
                } helper;

                helper.in = value;


                return std::move(helper.out);
            }
        }


        //This value is to be used when the condition returns false or doesn't exist.
        double defValue = 0;
        RE::SCRIPT_FUNCTION* function = nullptr;

        void Dispatch(RuntimeVariable& ret, Variable* target, std::span<Variable*>& args, ProcedureData& data) override
        {
            auto arg1 = GetArgFromParam<P1>(0, args);
            auto arg2 = GetArgFromParam<P2>(1, args);

            double result = 0;

            auto condition = function->conditionFunction;

            RE::TESObjectREFR* a_this = Unvariable<RE::TESObjectREFR*>{}(target);

            if (a_this && condition) {
                if (condition(a_this, Send(arg1), Send(arg2), result) == false) {
                    result = defValue;
                }
            }

            ret = result;
        }




        

    };
    static bool Create(IFunction* dispatchee)
    {
        std::unique_ptr<Dispatcher> test_dispatch = std::make_unique<ObScriptDispatcher<>>(nullptr);

        bool result = Dispatcher::TryRegister(test_dispatch.get(), dispatchee);

        if (result)  //If it's successful, we don't need to delete the dispatcher.
            test_dispatch.release();

        return result;
    }




    //template <size_t I = 0, RE::SCRIPT_PARAM_TYPE P1 = (RE::SCRIPT_PARAM_TYPE)0, RE::SCRIPT_PARAM_TYPE P2 = (RE::SCRIPT_PARAM_TYPE)0>
    std::unique_ptr<Dispatcher> CreateObscriptDispatcher(RE::SCRIPT_FUNCTION* function)
    {
        int param_count = function->numParams;

        if (!function->conditionFunction || param_count > 2) {
            return nullptr;
        }

        if (!param_count) {
            return std::make_unique<ObScriptDispatcher<>>(function);
        }


        RE::SCRIPT_PARAMETER& param = function->params[0];


        std::unique_ptr<Dispatcher> result;
        BinarySearch<(int)PARAM_TOTAL>(param.paramType.underlying(), [&]<size_t First> [[msvc::force_inline]] {
            constexpr auto P1 = (RE::SCRIPT_PARAM_TYPE)First;
            
            if (IsParamTypeAllowed(P1) == false) {
                return;
            }

            if (param_count < 2) {
                result = std::make_unique<ObScriptDispatcher<P1>>(function);
            }
            else {
                RE::SCRIPT_PARAMETER& param = function->params[1];

                return BinarySearch<(int)PARAM_TOTAL>(param.paramType.underlying(), [&]<size_t Second> [[msvc::force_inline]] {
                    constexpr auto P2 = (RE::SCRIPT_PARAM_TYPE)Second;

                    if (IsParamTypeAllowed(P1) == false) {
                        return;
                    }

                    result = std::make_unique<ObScriptDispatcher<P1, P2>>(function);
                });
            }
        });

        return result;
    }


   
    bool RegisterObScript(RE::SCRIPT_FUNCTION* obs_func, std::string_view path)
    {
        SignatureData sign{};



        sign.result = QualifiedType{ common_type::double_t() };
        sign.target = QualifiedType{ GetVariableType<RE::TESObjectREFR*>() };

        for (int i = 0; i < obs_func->numParams; i++) {
            RE::SCRIPT_PARAMETER& param = obs_func->params[i];

            sign.parameters.push_back(AnnotatedType{ GetTypeFromParam(*param.paramType) });

            //base.parameters.push_back()
        }
        
        IFunction* func = DirectoryManager::instance->GetFunctionFromPath(path, sign);

        if (!func) {
            return false;
        }

        std::unique_ptr<Dispatcher> dispatcher = CreateObscriptDispatcher(obs_func);

        if (!dispatcher) {
            return false;
        }

        bool result = Dispatcher::TryRegister(dispatcher.get(), func);

        if (result)  //If it's successful, we don't need to delete the dispatcher.
            dispatcher.release();

        return result;
    }







    INITIALIZE("function_register")
    {
        RegisterDump dump{ "Test" };

        dump = RegisterObScript(RE::SCRIPT_FUNCTION::LocateScriptCommand("GetGold"), "Shared::Obscript::GetGold");
        dump = RegisterObScript(RE::SCRIPT_FUNCTION::LocateScriptCommand("GetWithinDistance"), "Shared::Obscript::GetWithinDistance");
    }




    /////Misc stat shit

    enum struct MiscStatCategory
    {
        General,
        Quest,
        Combat,
        Magic,
        Crafting,
        Crime,
        Hidden,//If it has a name it's currently unknown to me -nb
    };








    //GetEventData
    enum struct EventFunctionType
    {
        GetIsID,    //Reference/Form/Location/Keyword
        IsInList,   //Reference/Form/Location/Keyword
        GetValue,
        HasKeyword,
        GetItemValue
    };

    enum class DATA_TYPE
    {
        Reference,
        Form,
        Location,
        Integer,
        Keyword,
    };


    double GetEventData(StaticTargetTag, EventFunctionType func_type, std::string_view value, RE::TESForm* data = nullptr)
    {
        //TLS + 1616/0x650 is The current BGSStoryEvent
        RE::TLSData* tls = RE::GetStaticTLSData();

        RE::BGSStoryEvent* event = REL::RelocateMember<RE::BGSStoryEvent*>(tls, 0x650);

        double result = 0;

        if (!event)
            return result;


        uint32_t member_code = 0;


        auto story_manager = RE::BGSStoryEventManager::GetSingleton();

        RE::BGSRegisteredStoryEvent& story_reg = story_manager->registeredEvents[event->index];

        if (story_reg.members) {
            for (RE::BGSStoryEventMember& member : *story_reg.members)
            {
                if (member.name == value) {
                    //These are a mismatch, and would never be allowed.
                    switch (func_type)
                    {
                    case EventFunctionType::GetValue:
                        if (member.type.underlying() != std::to_underlying(DATA_TYPE::Integer)) {
                            return 0;
                        }
                    default:
                        if (member.type.underlying() == std::to_underlying(DATA_TYPE::Integer)) {
                            return 0;
                        }
                    }



                    member_code = member.uniqueID;
                    break;
                }
            }
        }

        if (member_code) {
            static RE::SCRIPT_FUNCTION* script = RE::SCRIPT_FUNCTION::LocateScriptCommand("GetEventData");

            union
            {
                struct
                {
                    uint16_t unpack1;//High
                    uint16_t unpack2;
                };

                uint32_t packed{};
                void* value;

            } send{};

            send.unpack1 = (uint16_t)member_code;
            send.unpack2 = (uint16_t)func_type;

            if (script->conditionFunction(RE::PlayerCharacter::GetSingleton(), send.value, data, result) == 0) {
                //Should report error, but nothing really matters
            }
        }

        return result;
    }

    struct EventMember
    {
        enum DATA_TYPE
        {
            Reference,
            Form = 1,
            Location = 2,
            Integer = 3,
            Keyword = 4
        };


        int32_t unk0;
        int32_t codeMaybe;
        RE::BSString t;

    };



    


    double GetNumericPackageData(StaticTargetTag, std::string_view value)
    {
        //TLS + 0x640 is The current BGSPackageDataList
        RE::TLSData* tls = RE::GetStaticTLSData();

        RE::BGSPackageDataList* list = REL::RelocateMember<RE::BGSPackageDataList*>(tls, 0x640);

        double result = 0;

        if (!list)
            return result;


        return result;
    }


    double GetNumericPackageDataFromTarget(StaticTargetTag, EventFunctionType func_type, std::string_view value, RE::TESForm* data = nullptr)
    {
        //This one should target an object reference, getting the package data from one thing specifically
        return 0;
    }
#endif

}