// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsTypes.h"
#include "Menu.generated.h"

class UCreateLobbyWidget;
class ULobbyListWidget;
class UPasswordInputWidget;
class UWidgetSwitcher;
class UButton;

/**
 * Main menu widget that orchestrates all lobby UI
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(
		int32 NumberOfPublicConnections = 4,
		FString TypeOfMatch = FString(TEXT("FreeForAll")),
		FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby"))
	);

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	// Callbacks for the custom delegates on the Multiplayer Sessions Subsystem
	/* LOBBY CALLBACKS */
	UFUNCTION()
	void OnCreateLobby(bool bWasSuccessful, const FLobbyInfo& LobbyInfo);

	UFUNCTION()
	void OnPlayerLeft(const FLobbyPlayerInfo& PlayerInfo, ELobbyLeaveReason LeaveReason);

	UFUNCTION()
	void OnKickedFromLobby(FString Reason);

	UFUNCTION()
	void OnLobbyListUpdated(const TArray<FLobbyInfo>& LobbyList, bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerJoinedLobby(const FLobbyPlayerInfo& PlayerInfo);

	/* DEPRECATED SESSION CALLBACKS */
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	// Main menu buttons
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	// New widget references
	UPROPERTY(meta = (BindWidget))
	UCreateLobbyWidget* CreateLobbyWidget;

	UPROPERTY(meta = (BindWidget))
	ULobbyListWidget* LobbyListWidget;

	UPROPERTY(meta = (BindWidget))
	UPasswordInputWidget* PasswordInputWidget;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MenuSwitcher;

	// View indices for WidgetSwitcher
	enum class EMenuView : uint8
	{
		MainMenu = 0, // Host/Join Buttons
		CreateLobby = 1, // Lobby Creation Form
		LobbyBrowser = 2 // Lobby List
	};

	// Button Handlers
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	// Widget callback handlers
	UFUNCTION()
	void OnLobbyCreationComplete(bool bSuccess);

	UFUNCTION()
	void OnLobbySelected(const FLobbyInfo& SelectedLobby);

	UFUNCTION()
	void OnBackButtonPressed();

	UFUNCTION()
	void OnPasswordSubmitted(const FLobbyInfo& LobbyInfo, const FString& Password);

	UFUNCTION()
	void OnPasswordCancelled();

	// Join Flow Methods
	void JoinSelectedLobby(const FLobbyInfo& LobbyInfo, const FString& Password);

	UFUNCTION()
	void OnLobbyJoinComplete(ELobbyJoinResult Result);

	// Travel Method
	void TravelToLobby();

	// View Switching
	void SwitchToView(EMenuView View);

	// Utility
	void MenuTearDown();
	void PrintDebugMessage(const FString& Message, bool isError, const FColor Color = FColor::Green);

	// The subsystem designed to handle all online session functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	// State
	UPROPERTY()
	FLobbyInfo CurrentSelectedLobby;

	// Lobby Settings
	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{TEXT("")};
};
