// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
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
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.
		                              AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		MultiplayerSessionsSubsystem->MultiplayerOnPlayerLeftLobby.AddDynamic(this, &ThisClass::OnPlayerLeft);
	}
}

void UMenu::NativeDestruct()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.RemoveDynamic(
			this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.RemoveAll(this);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.RemoveAll(this);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(
			this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.RemoveDynamic(this, &ThisClass::OnStartSession);
		MultiplayerSessionsSubsystem->MultiplayerOnPlayerLeftLobby.RemoveDynamic(this, &ThisClass::OnPlayerLeft);
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

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		PrintDebugMessage(FString(TEXT("Session created successfully!")), false, FColor::Orange);

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

				// Steam complains about bUseLobbiesIfAvailable and bUsesPresence must match
				// so changing these in the results, which should NOT be needed!
				// Joining fails otherwise, though, so doing it for now.
				Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
				Result.Session.SessionSettings.bUsesPresence = true;

				MultiplayerSessionsSubsystem->JoinSession(Result);
				return;
			}
		}
	}
	else
	{
		if (GEngine)
		{
			PrintDebugMessage(
				FString(TEXT("Find Sessions Failed!")),
				true);
		}
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
			FString::Printf(TEXT("Found Session's Conntect String: %s"), *Address),
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
		PrintDebugMessage(FString(TEXT("Game is on!")), false, FColor::Emerald);
	}
	else
	{
		PrintDebugMessage(FString(TEXT("Game couldn't start.")), true);
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
	}
}

void UMenu::HostButtonClicked()
{
	if (MultiplayerSessionsSubsystem)
	{
		PrintDebugMessage(FString(TEXT("Host Button Clicked!")), false, FColor::Yellow);
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	if (MultiplayerSessionsSubsystem)
	{
		PrintDebugMessage(FString(TEXT("Join Button Clicked!")), false, FColor::Yellow);
		MultiplayerSessionsSubsystem->FindSessions(10000);
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
