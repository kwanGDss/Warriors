#pragma once
#include "Blueprint/BlueprintSupport.h"
#include "../Plugins/Online/OnlineSubsystemUtils/Source/OnlineSubsystemUtils/Classes/FindSessionsCallbackProxy.h"
#include "PlayerInfo.h"
#include "../Plugins/AdvancedSessions/Source/AdvancedSessions/Classes/BlueprintDataDefinitions.h"
class APlayerController;
class UGameInfoInstance_C1;
class UTexture2D;
#include "GameInfoInstance_BPI.generated.h"
UINTERFACE(Blueprintable, meta=(ReplaceConverted="/Game/Blueprints/AllLevels/GameInfoInstance_BPI.GameInfoInstance_BPI_C", OverrideNativeName="GameInfoInstance_BPI_C"))
class UGameInfoInstance_BPI_C : public UInterface
{
	GENERATED_BODY()
	static void __CustomDynamicClassInitialization(UDynamicClass* InDynamicClass) {}
};
class IGameInfoInstance_BPI_C
{
public:
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="ShowMainMenu"))
	void bpf__ShowMainMenu__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="ShowHostMenu"))
	void bpf__ShowHostMenu__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="ShowServerMenu"))
	void bpf__ShowServerMenu__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="ShowOptionMenu"))
	void bpf__ShowOptionMenu__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="JoinServer"))
	void bpf__JoinServer__pf(FBlueprintSessionResult bpp__SessionToJoin__pf);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="LaunchLobby"))
	void bpf__LaunchLobby__pf(int32 bpp__NumberOfPlayers__pf, bool bpp__UseLan__pf, const FText& bpp__ServerName__pf__const);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="ShowLoadingScreen"))
	void bpf__ShowLoadingScreen__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="DestroySessionCaller"))
	void bpf__DestroySessionCaller__pf(APlayerController* bpp__PlayerController__pf);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="SaveGameCheck"))
	void bpf__SaveGameCheck__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="ShowSessionMenu"))
	void bpf__ShowSessionMenu__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="GameInstanceRef"))
	void bpf__GameInstanceRef__pf(/*out*/ UGameInfoInstance_BPI_C*& bpp__GameInfoInstance__pf);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="SavePlayerInfo"))
	void bpf__SavePlayerInfo__pf(FPlayerInfo bpp__MyPlayerInfo__pf);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="MainMenuPlayerInfo"))
	void bpf__MainMenuPlayerInfo__pf();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="GetPlayerInfo"))
	void bpf__GetPlayerInfo__pf(/*out*/ FPlayerInfo& bpp__PlayerInfo__pf);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="UpdateFriendList"))
	void bpf__UpdateFriendList__pf(FBPFriendInfo bpp__FriendInfo__pf, UTexture2D* bpp__FriendAvatar__pf);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="Clear Friend Main Menu WB"))
	void bpf__ClearxFriendxMainxMenuxWB__pfTTTT();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta=(Category, OverrideNativeName="Set Game Setting"))
	void bpf__SetxGamexSetting__pfTT(int32 bpp__MapxTime__pfT);
public:
};
