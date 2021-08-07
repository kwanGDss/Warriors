#pragma once
#include "Blueprint/BlueprintSupport.h"
class AGameplay_PC_C;
#include "GameplayPC_BPI.generated.h"
UINTERFACE(Blueprintable, meta=(ReplaceConverted="/Game/Blueprints/Gameplay/GameplayPC_BPI.GameplayPC_BPI_C", OverrideNativeName="GameplayPC_BPI_C"))
class UGameplayPC_BPI_C : public UInterface
{
	GENERATED_BODY()
	static void __CustomDynamicClassInitialization(UDynamicClass* InDynamicClass) {}
};
class IGameplayPC_BPI_C
{
public:
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="Gameplay PC Ref"))
	void bpf__GameplayxPCxRef__pfTT(/*out*/ AGameplay_PC_C*& bpp__GameplayPC__pf);
public:
};
