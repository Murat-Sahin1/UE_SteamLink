// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Engine/LocalPlayer.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
// :
// CreateSessionCompleteDelegate(
// 	FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
// FindSessionsCompleteDelegate(
// 	FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
// JoinSessionCompleteDelegate(
// 	FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
// DestroySessionCompleteDelegate(
// 	FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
// StartSessionCompleteDelegate(
// 	FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	// IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	//
	// if (Subsystem)
	// {
	// 	SessionInterface = Subsystem->GetSessionInterface();
	// }
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();

		// Initialize the delegates
		CreateSessionCompleteDelegate =
			FOnCreateSessionCompleteDelegate::CreateUObject(
				this, &ThisClass::OnCreateSessionComplete
			);

		FindSessionsCompleteDelegate =
			FOnFindSessionsCompleteDelegate::CreateUObject(
				this, &ThisClass::OnFindSessionsComplete
			);

		JoinSessionCompleteDelegate =
			FOnJoinSessionCompleteDelegate::CreateUObject(
				this, &ThisClass::OnJoinSessionComplete
			);

		DestroySessionCompleteDelegate =
			FOnDestroySessionCompleteDelegate::CreateUObject(
				this, &ThisClass::OnDestroySessionComplete
			);

		StartSessionCompleteDelegate =
			FOnStartSessionCompleteDelegate::CreateUObject(
				this, &ThisClass::OnStartSessionComplete
			);
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConntections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	// Store the delegate in a FDelegateHandle 
	// so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	// Create Session Settings
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConntections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// Create Session
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool isCreateSessionSuccessful = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),
	                                                                 NAME_GameSession,
	                                                                 *LastSessionSettings);

	// If Create Session is not successful
	// Remove Create Session Delegate from the delegate list
	if (!isCreateSessionSuccessful)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

/* UTILITIES */

void UMultiplayerSessionsSubsystem::PrintDebugMessage(const FString& Message, bool isError)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			isError ? FColor::Red : FColor::Green,
			Message
		);
	}
}
