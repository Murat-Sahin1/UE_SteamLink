// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Engine/LocalPlayer.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
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
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConntections;
		LastMatchType = MatchType;
		DestroySession();
		return;
	}

	// Store the delegate in a FDelegateHandle 
	// so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	// Create Session Settings
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	LastSessionSettings->NumPublicConnections = NumPublicConntections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;

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

// Extending functionality to lobbies, creation of lobbies implemented.
void UMultiplayerSessionsSubsystem::CreateLobby(const FLobbySettings& LobbySettings)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnLobbyCreated.Broadcast(false, FLobbyInfo());
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		bCreateLobbyOnDestroy = true;
		PendingLobbySettings = LobbySettings;
		DestroySession();
		return;
	}

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		MultiplayerOnLobbyCreated.Broadcast(false, FLobbyInfo());
		return;
	}

	bIsLobbyOperation = true;
	PendingLobbySettings = LobbySettings;

	// Register Delegate
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	// Configure session settings
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	LastSessionSettings->NumPublicConnections = LobbySettings.MaxPlayers;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = false;
	LastSessionSettings->bAllowJoinViaPresence = false;
	LastSessionSettings->bUseLobbiesIfAvailable = true;

	// Lobby Metadata
	LastSessionSettings->Set(FName("LobbyIsPublic"), LobbySettings.bIsPublic,
	                         EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// Store password hash for private lobbies
	if (!LobbySettings.bIsPublic && !LobbySettings.Password.IsEmpty())
	{
		FString PasswordHash = HashPassword(LobbySettings.Password);
		LastSessionSettings->Set(FName("PasswordHash"), PasswordHash,
		                         EOnlineDataAdvertisementType::ViaOnlineService);
	}

	// Store host name into metadata
	FString HostName = LocalPlayer->GetNickname();
	LastSessionSettings->Set(FName("HostName"), HostName,
	                         EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// Create Lobby
	bool bSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),
	                                                NAME_GameSession, *LastSessionSettings);

	if (!bSuccess)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		bIsLobbyOperation = false;
		MultiplayerOnLobbyCreated.Broadcast(false, FLobbyInfo());
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	// Check if the Session Interface is valid
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// Add FindSessionsCompleteDelegate to the Session Interface Delegate List
	// Then get the delegate handle pointer
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FindSessionsCompleteDelegate);

	// Adjust the FindSessions Session Search Settings
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	// Get LocalPlayer for UniqeNetId
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// Trigger Find Sessions
	bool bWasSuccessful = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),
	                                                     LastSessionSearch.ToSharedRef());

	// Remove FindSessionsCompleteDelegateHandle from the Session Interface Delegate List
	// If the Find Sessions fails
	if (!bWasSuccessful)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bWasSuccessful = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),
	                                                    NAME_GameSession,
	                                                    SessionResult);
	if (!bWasSuccessful)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		DestroySessionCompleteDelegate);

	bool bWasSuccessful = SessionInterface->DestroySession(NAME_GameSession);
	if (!bWasSuccessful)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->
		AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	const bool bWasSuccessful = SessionInterface->StartSession(NAME_GameSession);
	if (!bWasSuccessful)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	// Handle lobby creation vs session creation
	if (bIsLobbyOperation)
	{
		bIsLobbyOperation = false;
		if (bWasSuccessful)
		{
			FLobbyInfo LobbyInfo = CreateLobbyInfoFromSession();
			MultiplayerOnLobbyCreated.Broadcast(true, LobbyInfo);
		}
		else
		{
			MultiplayerOnLobbyCreated.Broadcast(false, FLobbyInfo());
		}
	}
	else
	{
		// Original behavior for CreateSession
		MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
	}
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, true);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (SessionInterface)
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			CachedConnectAddress = Address;
		}

		MultiplayerOnJoinSessionComplete.Broadcast(Result);
	}
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		if (bCreateSessionOnDestroy)
		{
			bCreateSessionOnDestroy = false;
			CreateSession(LastNumPublicConnections, LastMatchType);
			return;
		}

		if (bCreateLobbyOnDestroy)
		{
			bCreateLobbyOnDestroy = false;
			CreateLobby(PendingLobbySettings);
			return;
		}
	}

	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
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

/* LOBBY UTILITIES */
FString UMultiplayerSessionsSubsystem::HashPassword(const FString& Password) const
{
	// Simple hash using MD5 - sufficient for lobby passwords
	return FMD5::HashAnsiString(*Password);
}

FLobbyInfo UMultiplayerSessionsSubsystem::CreateLobbyInfoFromSession() const
{
	FLobbyInfo Info;

	if (!SessionInterface.IsValid())
	{
		return Info;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!Session)
	{
		return Info;
	}

	Info.LobbyId = Session->GetSessionIdStr();
	Info.MaxPlayerCount = Session->SessionSettings.NumPublicConnections;
	Info.CurrentPlayerCount = Info.MaxPlayerCount - Session->NumOpenPublicConnections;

	// Retrieve stored metadata
	Session->SessionSettings.Get(FName("HostName"), Info.HostName);
	Session->SessionSettings.Get(FName("LobbyIsPublic"), Info.bIsPublic);

	// Host doesn't have ping to self
	Info.PingInMs = -1;

	return Info;
}
