#pragma once
#include "Blueprint/BlueprintSupport.h"
class AGameplay_GM_C;
#include "GameplayGM_BPI.generated.h"
UINTERFACE(Blueprintable, meta=(ReplaceConverted="/Game/Blueprints/Gameplay/GameplayGM_BPI.GameplayGM_BPI_C", OverrideNativeName="GameplayGM_BPI_C"))
class UGameplayGM_BPI_C : public UInterface
{
	GENERATED_BODY()
	static void __CustomDynamicClassInitialization(UDynamicClass* InDynamicClass) {}
};
class IGameplayGM_BPI_C
{
public:
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="Gameplay GM Ref"))
	void bpf__GameplayxGMxRef__pfTT(/*out*/ AGameplay_GM_C*& bpp__GameplayxGMxRef__pfTT);
public:
};
