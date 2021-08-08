#pragma once
#include "Blueprint/BlueprintSupport.h"
#include "PlayerInfo.h"
#include "GameplayGM_BPI.h"
#include "GameInfoInstance_BPI.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "GameplayPC_BPI.h"
class UUserWidget;
class AGameplay_GM_C;
class AGameplay_PC_C;
#include "Gameplay_PC.generated.h"
UCLASS(config=Game, Blueprintable, BlueprintType, meta=(ReplaceConverted="/Game/Blueprints/Gameplay/Gameplay_PC.Gameplay_PC_C", OverrideNativeName="Gameplay_PC_C"))
class AGameplay_PC_C : public APlayerController, public IGameplayPC_BPI_C
{
public:
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="Player Setting Save", Category="Default", MultiLine="true", OverrideNativeName="PlayerSettingSave"))
	FString bpv__PlayerSettingSave__pf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, meta=(DisplayName="Player Setting", Category="Default", MultiLine="true", OverrideNativeName="PlayerSetting"))
	FPlayerInfo__pf533497531 bpv__PlayerSetting__pf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, meta=(DisplayName="Game Play Status", Category="Default", MultiLine="true", ExposeOnSpawn="true", OverrideNativeName="GamePlay Status"))
	UUserWidget* bpv__GamePlayxStatus__pfT;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGameplay_GM_BPI"))
	TScriptInterface<IGameplayGM_BPI_C> b0l__K2Node_DynamicCast_AsGameplay_GM_BPI__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess"))
	bool b0l__K2Node_DynamicCast_bSuccess__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_AsGame_Info_Instance_BPI"))
	TScriptInterface<IGameInfoInstance_BPI_C> b0l__K2Node_DynamicCast_AsGame_Info_Instance_BPI__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="K2Node_DynamicCast_bSuccess_1"))
	bool b0l__K2Node_DynamicCast_bSuccess_1__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_Gameplay_GM_Ref_Gameplay_GM_Ref"))
	AGameplay_GM_C* b0l__CallFunc_Gameplay_GM_Ref_Gameplay_GM_Ref__pf;
	UPROPERTY(Transient, DuplicateTransient, meta=(OverrideNativeName="CallFunc_GetPlayerInfo_PlayerInfo"))
	FPlayerInfo__pf533497531 b0l__CallFunc_GetPlayerInfo_PlayerInfo__pf;
	AGameplay_PC_C(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override;
	static void __CustomDynamicClassInitialization(UDynamicClass* InDynamicClass);
	static void __StaticDependenciesAssets(TArray<FBlueprintDependencyData>& AssetsToLoad);
	static void __StaticDependencies_DirectlyUsedAssets(TArray<FBlueprintDependencyData>& AssetsToLoad);
	void bpf__ExecuteUbergraph_Gameplay_PC__pf_0(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_PC__pf_1(int32 bpp__EntryPoint__pf);
	void bpf__ExecuteUbergraph_Gameplay_PC__pf_2(int32 bpp__EntryPoint__pf);
	UFUNCTION(meta=(Comment="/** Event when play begins for this actor. */", DisplayName="BeginPlay", ToolTip="Event when play begins for this actor.", CppFromBpEvent, OverrideNativeName="ReceiveBeginPlay"))
	virtual void bpf__ReceiveBeginPlay__pf();
	UFUNCTION(Server, Reliable, BlueprintCallable, meta=(Category, OverrideNativeName="SetupHUDWindow"))
	virtual void bpf__SetupHUDWindow__pf();
	UFUNCTION(Server, Reliable, BlueprintCallable, meta=(Category, OverrideNativeName="PassCharacterInfoToServer"))
	virtual void bpf__PassCharacterInfoToServer__pf();
	UFUNCTION(BlueprintCallable, meta=(Category, OverrideNativeName="LoadGame"))
	virtual void bpf__LoadGame__pf();
	UFUNCTION(BlueprintCallable, meta=(Category, CppFromBpEvent, OverrideNativeName="Gameplay PC Ref"))
	virtual void bpf__GameplayxPCxRef__pfTT(/*out*/ AGameplay_PC_C*& bpp__GameplayPC__pf);
public:
};
