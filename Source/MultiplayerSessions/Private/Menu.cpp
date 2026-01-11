// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "MultiplayerSessionsSubsystem.h"
#include "CreateLobbyWidget.h"
#include "LobbyListWidget.h"
#include "PasswordInputWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = LobbyPath + "?listen";
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeUIOnly;
			InputModeUIOnly.SetWidgetToFocus(TakeWidget());
			InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PlayerController->SetInputMode(InputModeUIOnly);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		/* LOBBY CALLBACKS */
		MultiplayerSessionsSubsystem->MultiplayerOnLobbyCreated.AddDynamic(this, &ThisClass::OnCreateLobby);
		MultiplayerSessionsSubsystem->MultiplayerOnPlayerLeftLobby.AddDynamic(this, &ThisClass::OnPlayerLeft);
		MultiplayerSessionsSubsystem->MultiplayerOnKickedFromLobby.AddDynamic(this, &ThisClass::OnKickedFromLobby);
		MultiplayerSessionsSubsystem->MultiplayerOnLobbyListUpdated.AddDynamic(this, &ThisClass::OnLobbyListUpdated);
		MultiplayerSessionsSubsystem->MultiplayerOnPlayerJoinedLobby.AddDynamic(this, &ThisClass::OnPlayerJoinedLobby);
		MultiplayerSessionsSubsystem->MultiplayerOnLobbyJoinComplete.AddDynamic(this, &ThisClass::OnLobbyJoinComplete);

		/* DEPRECATED DELEGATES */
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.
		                              AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

		// Setup child widgets
		if (CreateLobbyWidget)
		{
			CreateLobbyWidget->Setup(MultiplayerSessionsSubsystem);
			CreateLobbyWidget->OnLobbyCreationComplete.AddDynamic(this, &ThisClass::OnLobbyCreationComplete);
		}

		if (LobbyListWidget)
		{
			LobbyListWidget->Setup(MultiplayerSessionsSubsystem);
			LobbyListWidget->OnLobbySelected.AddDynamic(this, &ThisClass::OnLobbySelected);
			LobbyListWidget->OnBackButtonPressed.AddDynamic(this, &ThisClass::OnBackButtonPressed);
		}

		if (PasswordInputWidget)
		{
			PasswordInputWidget->OnPasswordSubmitted.AddDynamic(this, &ThisClass::OnPasswordSubmitted);
			PasswordInputWidget->OnPasswordCancelled.AddDynamic(this, &ThisClass::OnPasswordCancelled);
		}

		// Start on main menu view
		SwitchToView(EMenuView::MainMenu);
	}
}

void UMenu::NativeDestruct()
{
	if (MultiplayerSessionsSubsystem)
	{
		// Unbind lobby delegates
		MultiplayerSessionsSubsystem->MultiplayerOnLobbyCreated.RemoveDynamic(this, &ThisClass::OnCreateLobby);
		MultiplayerSessionsSubsystem->MultiplayerOnPlayerLeftLobby.RemoveDynamic(this, &ThisClass::OnPlayerLeft);
		MultiplayerSessionsSubsystem->MultiplayerOnKickedFromLobby.RemoveDynamic(this, &ThisClass::OnKickedFromLobby);
		MultiplayerSessionsSubsystem->MultiplayerOnLobbyListUpdated.RemoveDynamic(this, &ThisClass::OnLobbyListUpdated);
		MultiplayerSessionsSubsystem->MultiplayerOnPlayerJoinedLobby.RemoveDynamic(
			this, &ThisClass::OnPlayerJoinedLobby);
		MultiplayerSessionsSubsystem->MultiplayerOnLobbyJoinComplete.RemoveDynamic(
			this, &ThisClass::OnLobbyJoinComplete);

		// Deprecated
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.RemoveDynamic(
			this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.RemoveAll(this);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.RemoveAll(this);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(
			this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.RemoveDynamic(this, &ThisClass::OnStartSession);
	}

	// Unbind widget delegates
	if (CreateLobbyWidget)
	{
		CreateLobbyWidget->OnLobbyCreationComplete.RemoveDynamic(this, &ThisClass::OnLobbyCreationComplete);
	}

	if (LobbyListWidget)
	{
		LobbyListWidget->OnLobbySelected.RemoveDynamic(this, &ThisClass::OnLobbySelected);
		LobbyListWidget->OnBackButtonPressed.RemoveDynamic(this, &ThisClass::OnBackButtonPressed);
	}

	if (PasswordInputWidget)
	{
		PasswordInputWidget->OnPasswordSubmitted.RemoveDynamic(this, &ThisClass::OnPasswordSubmitted);
		PasswordInputWidget->OnPasswordCancelled.RemoveDynamic(this, &ThisClass::OnPasswordCancelled);
	}

	MenuTearDown();
	Super::NativeDestruct();
}

bool UMenu::Initialize()
{
	bool initializedParent = Super::Initialize();
	if (!initializedParent)
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

// Button Handlers
void UMenu::HostButtonClicked()
{
	PrintDebugMessage(FString(TEXT("Host Button Clicked!")), false, FColor::Yellow);
	SwitchToView(EMenuView::CreateLobby);
}

void UMenu::JoinButtonClicked()
{
	PrintDebugMessage(FString(TEXT("Join Button Clicked!")), false, FColor::Yellow);
	SwitchToView(EMenuView::LobbyBrowser);

	// Trigger lobby search
	if (LobbyListWidget)
	{
		LobbyListWidget->RefreshLobbyList();
	}
}

// Widget Callback Handlers
void UMenu::OnLobbyCreationComplete(bool bSuccess)
{
	if (bSuccess)
	{
		// Lobby created successfully - server travel happens in OnCreateLobby callback
		PrintDebugMessage(TEXT("Navigating to lobby..."), false, FColor::Green);
	}
	else
	{
		// User cancelled or creation failed - return to main menu
		SwitchToView(EMenuView::MainMenu);
	}
}

void UMenu::OnLobbySelected(const FLobbyInfo& SelectedLobby)
{
	CurrentSelectedLobby = SelectedLobby;

	if (SelectedLobby.bIsPublic)
	{
		// Public lobby - join directly without password
		JoinSelectedLobby(SelectedLobby, TEXT(""));
	}
	else
	{
		// Private lobby - show password dialog
		if (PasswordInputWidget)
		{
			PasswordInputWidget->ShowForLobby(SelectedLobby);
		}
	}
}

void UMenu::OnBackButtonPressed()
{
	SwitchToView(EMenuView::MainMenu);
}

void UMenu::OnPasswordSubmitted(const FLobbyInfo& LobbyInfo, const FString& Password)
{
	JoinSelectedLobby(LobbyInfo, Password);
}

void UMenu::OnPasswordCancelled()
{
	// User cancelled password input - stay on lobby browser
	PrintDebugMessage(TEXT("Password entry cancelled"), false, FColor::Yellow);
}

// Join Flow Methods
void UMenu::JoinSelectedLobby(const FLobbyInfo& LobbyInfo, const FString& Password)
{
	if (!MultiplayerSessionsSubsystem)
	{
		PrintDebugMessage(TEXT("Session subsystem not available"), true);
		return;
	}

	PrintDebugMessage(
		FString::Printf(TEXT("Attempting to join %s's lobby..."), *LobbyInfo.HostName),
		false, FColor::Cyan);

	// Call subsystem to join lobby
	MultiplayerSessionsSubsystem->JoinLobby(LobbyInfo, Password);
}

void UMenu::OnLobbyJoinComplete(ELobbyJoinResult Result)
{
	switch (Result)
	{
	case ELobbyJoinResult::Success:
		PrintDebugMessage(TEXT("Successfully joined lobby!"), false, FColor::Green);

		// Hide password dialog if it's open
		if (PasswordInputWidget)
		{
			PasswordInputWidget->Hide();
		}

		// Travel to lobby level
		TravelToLobby();
		break;

	case ELobbyJoinResult::WrongPassword:
		PrintDebugMessage(TEXT("Incorrect password"), true);

		// Keep password dialog open, show error
		if (PasswordInputWidget)
		{
			PasswordInputWidget->ShowError(TEXT("Incorrect password. Please try again."));
		}
		break;

	case ELobbyJoinResult::LobbyFull:
		PrintDebugMessage(TEXT("Lobby is full"), true);

		// Hide password dialog
		if (PasswordInputWidget)
		{
			PasswordInputWidget->Hide();
		}
		break;

	case ELobbyJoinResult::LobbyNotFound:
		PrintDebugMessage(TEXT("Lobby no longer exists"), true);

		// Hide password dialog
		if (PasswordInputWidget)
		{
			PasswordInputWidget->Hide();
		}

		// Refresh lobby list
		if (LobbyListWidget)
		{
			LobbyListWidget->RefreshLobbyList();
		}
		break;

	case ELobbyJoinResult::ConnectionFailed:
	case ELobbyJoinResult::UnknownError:
	default:
		PrintDebugMessage(TEXT("Failed to connect to lobby"), true);

		// Hide password dialog
		if (PasswordInputWidget)
		{
			PasswordInputWidget->Hide();
		}
		break;
	}
}

// Travel Lobby Method
void UMenu::TravelToLobby()
{
	if (!MultiplayerSessionsSubsystem)
	{
		PrintDebugMessage(TEXT("Cannot travel - subsystem not available"), true);
		return;
	}

	FString ConnectAddress = MultiplayerSessionsSubsystem->GetCachedConnectAddress();

	if (ConnectAddress.IsEmpty())
	{
		PrintDebugMessage(TEXT("Failed to get server address"), true);
		return;
	}

	PrintDebugMessage(
		FString::Printf(TEXT("Traveling to: %s"), *ConnectAddress),
		false, FColor::Purple);

	// Clean up menu
	MenuTearDown();

	// Travel as client
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(ConnectAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}

// View Switching
void UMenu::SwitchToView(EMenuView View)
{
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(View));
	}
}

void UMenu::OnCreateLobby(bool bWasSuccessful, const FLobbyInfo& LobbyInfo)
{
	if (bWasSuccessful)
	{
		PrintDebugMessage(FString(TEXT("Lobby created successfully!")), false, FColor::Purple);

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
			PrintDebugMessage(
				FString::Printf(
					TEXT(
						"HostName: %s, "
						"LobbyId: %s, "
						"CurrentPlayerCount: %d, "
						"MaxPlayerCount: %d, "
						"PingInMs: %d"),
					*LobbyInfo.HostName,
					*LobbyInfo.LobbyId,
					LobbyInfo.CurrentPlayerCount,
					LobbyInfo.MaxPlayerCount,
					LobbyInfo.PingInMs),
				false);
		}
	}
	else
	{
		PrintDebugMessage(FString(TEXT("Failed to create lobby!")), true);
	}
}

void UMenu::OnLobbyListUpdated(const TArray<FLobbyInfo>& LobbyList, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	if (bWasSuccessful)
	{
		if (LobbyList.Num() == 0)
		{
			PrintDebugMessage(TEXT("No lobbies found."), false, FColor::Yellow);
			return;
		}

		PrintDebugMessage(
			FString::Printf(TEXT("Found %d lobby(s):"), LobbyList.Num()),
			false, FColor::Green);

		for (int32 i = 0; i < LobbyList.Num(); i++)
		{
			const FLobbyInfo& Lobby = LobbyList[i];
			PrintDebugMessage(
				FString::Printf(
					TEXT("[%d] Host: %s | Players: %d/%d | Ping: %dms | %s"),
					i + 1,
					*Lobby.HostName,
					Lobby.CurrentPlayerCount,
					Lobby.MaxPlayerCount,
					Lobby.PingInMs,
					Lobby.bIsPublic ? TEXT("Public") : TEXT("Private")),
				false, FColor::Cyan);
		}
	}
	else
	{
		PrintDebugMessage(TEXT("Find Lobbies failed."), true);
	}
}

void UMenu::OnPlayerLeft(const FLobbyPlayerInfo& PlayerInfo, ELobbyLeaveReason LeaveReason)
{
	switch (LeaveReason)
	{
	case ELobbyLeaveReason::Kicked:
		{
			PrintDebugMessage(
				FString::Printf(
					TEXT("%s is kicked!"), *PlayerInfo.PlayerName),
				false);
			break;
		}
	case ELobbyLeaveReason::Left:
		{
			PrintDebugMessage(
				FString::Printf(
					TEXT("%s has left the game!"), *PlayerInfo.PlayerName),
				false);
			break;
		}
	default:
		break;
	}
}

void UMenu::OnKickedFromLobby(FString Reason)
{
	PrintDebugMessage(FString("You have been kicked from the lobby"), false);
}

void UMenu::OnPlayerJoinedLobby(const FLobbyPlayerInfo& PlayerInfo)
{
	if (!PlayerInfo.PlayerName.IsEmpty())
	{
		PrintDebugMessage(
			FString::Printf(
				TEXT(
					"New Player Joined!, \n"
					"PlayerId: %s, "
					"PlayerName: %s, "),
				*PlayerInfo.PlayerId,
				*PlayerInfo.PlayerName),
			false, FColor::Yellow);
	}
}

void UMenu::MenuTearDown()
{
	// RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeGameOnly;
			PlayerController->SetInputMode(InputModeGameOnly);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

/* UTILITIES */

void UMenu::PrintDebugMessage(const FString& Message, bool isError, const FColor Color)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			isError ? FColor::Red : Color,
			Message
		);
	}
}

/* DEPRECATED SESSION CALLBACKS */
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		PrintDebugMessage(FString(TEXT("Session created successfully!")), false, FColor::Yellow);

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		PrintDebugMessage(FString(TEXT("Failed to create session!")), true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	if (bWasSuccessful)
	{
		for (auto Result : SessionResults)
		{
			FString SettingsValue;
			FString Id = Result.GetSessionIdStr();
			FString User = Result.Session.OwningUserName;

			Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

			if (SettingsValue == MatchType)
			{
				PrintDebugMessage(
					FString::Printf(
						TEXT("Found Session Details: ID: %s, Host User: %s"), *Id, *User),
					false);

				Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
				Result.Session.SessionSettings.bUsesPresence = true;

				MultiplayerSessionsSubsystem->JoinSession(Result);
				return;
			}
		}
	}
	else
	{
		PrintDebugMessage(
			FString(TEXT("Find Sessions Failed!")),
			true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (MultiplayerSessionsSubsystem == nullptr)
		{
			return;
		}

		FString Address = MultiplayerSessionsSubsystem->GetCachedConnectAddress();
		PrintDebugMessage(
			FString::Printf(TEXT("Found Session's Connect String: %s"), *Address),
			false);

		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			APlayerController* FirstLocalPlayerController = GameInstance->GetFirstLocalPlayerController();
			if (FirstLocalPlayerController)
			{
				FirstLocalPlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
	if (bWasSuccessful)
		PrintDebugMessage(FString(TEXT("Destroyed a Session!")), false, FColor::Yellow);
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		PrintDebugMessage(FString(TEXT("Game is on!")), false, FColor::Yellow);
	}
	else
	{
		PrintDebugMessage(FString(TEXT("Game couldn't start.")), true);
	}
}
