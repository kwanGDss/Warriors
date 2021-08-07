#pragma once
#include "Blueprint/BlueprintSupport.h"
#include "PlayerInfo.h"
#include "GameplayPC_BPI.h"
#include "Runtime/CoreUObject/Public/UObject/NoExportTypes.h"
#include "GameInfoInstance_BPI.h"
#include "Runtime/Engine/Classes/GameFramework/GameModeBase.h"
#include "GameplayGM_BPI.h"
class USceneComponent;
class APlayerController;
class APlayerStart;
class AGameplay_PC_C;
class ACharacter;
class UClass;
class AController;
class UGameInfoInstance_C;
class AGameplay_GM_C;
#include "Gameplay_GM.generated.h"
UCLASS(config=Game, Blueprintable, BlueprintType, meta=(ReplaceConverted="/Game/Blueprints/Gameplay/Gameplay_GM.Gameplay_GM_C", OverrideNativeName="Gameplay_GM_C"))
class AGameplay_GM_C : public AGameModeBase, public IGameplayGM_BPI_C
{
public:
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, NonTransactional, meta=(Category="디폴트", OverrideNativeName="DefaultSceneRoot"))
	USceneComponent* bpv__DefaultSceneRoot__pf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, meta=(DisplayName="All Player Controller", Category="GameInfo", MultiLine="true", OverrideNativeName="AllPlayerController"))
	TArray<APlayerController*> bpv__AllPlayerController__pf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, meta=(DisplayName="Spawn Point", Category="GameInfo", MultiLine="true", OverrideNativeName="SpawnPoint"))
	TArray<APlayerStart*> bpv__SpawnPoint__pf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, meta=(DisplayName="Stamina", Category="Default", MultiLine="true", ExposeOnSpawn="true", OverrideNativeName="Stamina"))
	float bpv__Stamina__pf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, meta=(DisplayName="Health", Category="Default", MultiLine="true", ExposeOnSpawn="true", OverrideNativeName="Health"))
	float bpv__Health__pf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="Connecting Player", Category="Default", MultiLine="true", OverrideNativeName="Connecting Player"))
	APlayerController* bpv__ConnectingxPlayer__pfT;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="Gameplay PC", Category="Default", MultiLine="true", OverrideNativeName="Gameplay PC"))
	AGameplay_PC_C* bpv__GameplayxPC__pfT;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="My Player Info", Category="Default", MultiLine="true", OverrideNativeName="MyPlayerInfo"))
	FPlayerInfo__pf533497531 bpv__MyPlayerInfo__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_Event_OldPC"))
	APlayerController* b0l__K2Node_Event_OldPC__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_Event_NewPC"))
	APlayerController* b0l__K2Node_Event_NewPC__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_CustomEvent_PlayerController"))
	APlayerController* b0l__K2Node_CustomEvent_PlayerController__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_CustomEvent_PlayerCharacter"))
	UClass* b0l__K2Node_CustomEvent_PlayerCharacter__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGameplay_PC_BPI"))
	TScriptInterface<IGameplayPC_BPI_C> b0l__K2Node_DynamicCast_AsGameplay_PC_BPI__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess"))
	bool b0l__K2Node_DynamicCast_bSuccess__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_Gameplay_PC_Ref_GameplayPC"))
	AGameplay_PC_C__pf1836565435* b0l__CallFunc_Gameplay_PC_Ref_GameplayPC__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_GetAllActorsOfClass_OutActors"))
	TArray<APlayerStart*> b0l__CallFunc_GetAllActorsOfClass_OutActors__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_Event_ExitingController"))
	AController* b0l__K2Node_Event_ExitingController__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGameplay_PC_BPI_1"))
	TScriptInterface<IGameplayPC_BPI_C> b0l__K2Node_DynamicCast_AsGameplay_PC_BPI_1__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess_1"))
	bool b0l__K2Node_DynamicCast_bSuccess_1__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_Gameplay_PC_Ref_GameplayPC_1"))
	AGameplay_PC_C__pf1836565435* b0l__CallFunc_Gameplay_PC_Ref_GameplayPC_1__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_Array_Get_Item"))
	APlayerStart* b0l__CallFunc_Array_Get_Item__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_GetTransform_ReturnValue"))
	FTransform b0l__CallFunc_GetTransform_ReturnValue__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGame_Info_Instance_BPI"))
	TScriptInterface<IGameInfoInstance_BPI_C> b0l__K2Node_DynamicCast_AsGame_Info_Instance_BPI__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess_2"))
	bool b0l__K2Node_DynamicCast_bSuccess_2__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_GameInstanceRef_GameInfoInstance"))
	UGameInfoInstance_C__pf533497531* b0l__CallFunc_GameInstanceRef_GameInfoInstance__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_DestroySessionCaller_self_CastInput"))
	TScriptInterface<IGameInfoInstance_BPI_C> b0l__CallFunc_DestroySessionCaller_self_CastInput__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_CustomEvent_Stamina"))
	float b0l__K2Node_CustomEvent_Stamina__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_CustomEvent_Health"))
	float b0l__K2Node_CustomEvent_Health__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_Event_NewPlayer"))
	APlayerController* b0l__K2Node_Event_NewPlayer__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGameplay_PC_BPI_2"))
	TScriptInterface<IGameplayPC_BPI_C> b0l__K2Node_DynamicCast_AsGameplay_PC_BPI_2__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess_3"))
	bool b0l__K2Node_DynamicCast_bSuccess_3__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_Gameplay_PC_Ref_GameplayPC_2"))
	AGameplay_PC_C__pf1836565435* b0l__CallFunc_Gameplay_PC_Ref_GameplayPC_2__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_GetAllActorsOfClass_OutActors_1"))
	TArray<APlayerStart*> b0l__CallFunc_GetAllActorsOfClass_OutActors_1__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGame_Info_Instance_BPI_1"))
	TScriptInterface<IGameInfoInstance_BPI_C> b0l__K2Node_DynamicCast_AsGame_Info_Instance_BPI_1__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess_4"))
	bool b0l__K2Node_DynamicCast_bSuccess_4__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_CustomEvent_Damage"))
	float b0l__K2Node_CustomEvent_Damage__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_GetPlayerInfo_PlayerInfo"))
	FPlayerInfo__pf533497531 b0l__CallFunc_GetPlayerInfo_PlayerInfo__pf;
	AGameplay_GM_C(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override;
	static void __CustomDynamicClassInitialization(UDynamicClass* InDynamicClass);
	static void __StaticDependenciesAssets(TArray<FBlueprintDependencyData>& AssetsToLoad);
	static void __StaticDependencies_DirectlyUsedAssets(TArray<FBlueprintDependencyData>& AssetsToLoad);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_0(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_1(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_2(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_3(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_4(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_5(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_6(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_GM__pf_7(int32 bpp__EntryPoint__pf);
	UFUNCTION(NetMulticast, BlueprintCallable, Unreliable, meta=(Category, OverrideNativeName="IncreaseStamina"))
	virtual void bpf__IncreaseStamina__pf();
	UFUNCTION(NetMulticast, BlueprintCallable, Unreliable, meta=(Category, OverrideNativeName="GetDamaged"))
	virtual void bpf__GetDamaged__pf(float bpp__Damage__pf);
	UFUNCTION(meta=(Category="Game", Comment="/** Notification that a player has successfully logged in, and has been given a player controller */", DisplayName="OnPostLogin", ScriptName="OnPostLogin", ToolTip="Notification that a player has successfully logged in, and has been given a player controller", CppFromBpEvent, OverrideNativeName="K2_PostLogin"))
	virtual void bpf__K2_PostLogin__pf(APlayerController* bpp__NewPlayer__pf);
	UFUNCTION(BlueprintCallable, meta=(Category, OverrideNativeName="Set Health"))
	virtual void bpf__SetxHealth__pfT(float bpp__Health__pf);
	UFUNCTION(BlueprintCallable, meta=(Category, OverrideNativeName="Set Stamia"))
	virtual void bpf__SetxStamia__pfT(float bpp__Stamina__pf);
	UFUNCTION(meta=(Category="Game", Comment="/** Implementable event when a Controller with a PlayerState leaves the game. */", DisplayName="OnLogout", ScriptName="OnLogout", ToolTip="Implementable event when a Controller with a PlayerState leaves the game.", CppFromBpEvent, OverrideNativeName="K2_OnLogout"))
	virtual void bpf__K2_OnLogout__pf(AController* bpp__ExitingController__pf);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, meta=(Category, OverrideNativeName="RespawnPlayer"))
	virtual void bpf__RespawnPlayer__pf(APlayerController* bpp__PlayerController__pf, UClass* bpp__PlayerCharacter__pf);
	UFUNCTION(meta=(Category="Game", Comment="/** Called when a PlayerController is swapped to a new one during seamless travel */", DisplayName="OnSwapPlayerControllers", ScriptName="OnSwapPlayerControllers", ToolTip="Called when a PlayerController is swapped to a new one during seamless travel", CppFromBpEvent, OverrideNativeName="K2_OnSwapPlayerControllers"))
	virtual void bpf__K2_OnSwapPlayerControllers__pf(APlayerController* bpp__OldPC__pf, APlayerController* bpp__NewPC__pf);
	UFUNCTION(BlueprintCallable, meta=(Category, CppFromBpEvent, OverrideNativeName="Gameplay GM Ref"))
	virtual void bpf__GameplayxGMxRef__pfTT(/*out*/ AGameplay_GM_C*& bpp__GameplayxGMxRef__pfTT);
public:
};
