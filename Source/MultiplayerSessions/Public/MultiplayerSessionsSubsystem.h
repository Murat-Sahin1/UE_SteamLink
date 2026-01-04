// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsTypes.h"
#include "MultiplayerSessionsSubsystem.generated.h"

// Declaring our own custom delegates for the Menu class to bind callbacks to
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete,
                                     const TArray<FOnlineSessionSearchResult>&,
                                     bool);

DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete,
                                    EOnJoinSessionCompleteResult::Type);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete,
                                            bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete,
                                            bool, bWasSuccessful);

// LOBBY SYSTEM DELEGATES
// ----------------------

// Lobby creation result
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnLobbyCreated,
                                             bool, bWasSuccessful,
                                             const FLobbyInfo&, LobbyInfo);

// Lobby search result
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnLobbyListUpdated,
                                             const TArray<FLobbyInfo>&, Lobbies,
                                             bool, bWasSuccessful);

// Lobby join attempt result
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnLobbyJoinComplete,
                                            ELobbyJoinResult, Result);

// Lobby player joined event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnPlayerJoinedLobby,
                                            const FLobbyPlayerInfo&, PlayerInfo);

// Lobby player left event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnPlayerLeftLobby,
                                             const FLobbyPlayerInfo&, PlayerInfo,
                                             ELobbyLeaveReason, Reason);

// Lobby local player kicked event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnKickedFromLobby,
                                            FString, Reason);

// Lobby host changed event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnHostMigration,
                                             const FLobbyPlayerInfo&, OldHost,
                                             const FLobbyPlayerInfo&, NewHost);

// Lobby settings modified event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnLobbySettingsUpdated,
                                            const FLobbyInfo&, UpdatedLobbyInfo);


// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnUnregisterPlayerComplete,
//                                              const FUniqueNetId&, PlayerId,
//                                              EOnSessionParticipantLeftReason, Reason);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();

	// LOBBY HANDLERS
	// -----------------------
	void CreateLobby(const FLobbySettings& Settings);
	void FindLobbies(int32 MaxResult = 100);
	void JoinLobby(const FLobbyInfo& LobbyInfo, const FString& Password = TEXT(""));
	void UpdateLobbySettings(const FLobbySettings& NewSettings);
	void SetLobbyVisibility(bool bIsPublic, const FString& Password = TEXT(""));
	void KickPlayer(const FString& PlayerId, const FString& Reason = TEXT(""));
	void TransferHost(const FString& NewHostPlayerId);

	// CUSTOM DELEGATES
	// -----------------------
	// Our own custom delegates, for the Menu class to bind callbacks to
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	// LOBBY DELEGATES
	// ----------------------
	FMultiplayerOnLobbyCreated MultiplayerOnLobbyCreated;
	FMultiplayerOnLobbyListUpdated MultiplayerOnLobbyListUpdated;
	FMultiplayerOnLobbyJoinComplete MultiplayerOnLobbyJoinComplete;
	FMultiplayerOnPlayerJoinedLobby MultiplayerOnPlayerJoinedLobby;
	FMultiplayerOnPlayerLeftLobby MultiplayerOnPlayerLeftLobby;
	FMultiplayerOnKickedFromLobby MultiplayerOnKickedFromLobby;
	FMultiplayerOnHostMigration MultiplayerOnHostMigration;
	FMultiplayerOnLobbySettingsUpdated MultiplayerOnLobbySettingsUpdated;

	// LOBBY QUERY METHODS
	// ------------------------
	FLobbyInfo GetCurrentLobbyInfo() const;
	TArray<FLobbyPlayerInfo> GetLobbyPlayers() const;
	FLobbyPlayerInfo GetLobbyPlayer(const FUniqueNetId& PlayerId) const;
	bool IsLobbyHost() const;
	bool IsInLobby() const;
	void LeaveLobby();

	// GETTER FUNCTIONS
	// -----------------------
	// Getter functions for the Menu Class
	const FString& GetCachedConnectAddress() const { return CachedConnectAddress; }

	/* DEPRECATED */
	// SESSION HANDLERS
	// ------------------------
	// These will be called outside the Multiplayer Sessions Subsystem
	void CreateSession(int32 NumPublicConntections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

protected:
	// LIFETIME OVERRIDES
	// ------------------------
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// CALLBACKS
	// ------------------------
	// Internal callbacks for the delegates
	// These will be called within this class
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnUnregisterPlayerComplete(FName SessionName, const FUniqueNetId& PlayerId,
	                                EOnSessionParticipantLeftReason Reason);

private:
	typedef UMultiplayerSessionsSubsystem ThisClass;

	// Global Pointers
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	FString CachedConnectAddress;

	// DELEGATES
	// ------------------------
	// To add ONLINE SESSION INTERFACE DELEGATE LIST
	// Internal Multiplayer Sessions Subsystem callbacks will be bound to these delegates
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;
	FOnSessionParticipantLeftDelegate SessionParticipantLeftDelegate;
	FDelegateHandle SessionParticipantLeftDelegateHandle;

	// SESSION STATE
	// Soon will be deprecated
	// ------------------------
	bool bCreateSessionOnDestroy{false};
	int32 LastNumPublicConnections;
	FString LastMatchType;

	// LOBBY STATE
	// ------------------------
	bool bCreateLobbyOnDestroy{false};
	FLobbySettings PendingLobbySettings;
	bool bIsLobbyOperation{false};
	bool bIsLobbySearch{false};
	bool bIsLobbyJoin{false};
	TMap<FString, FString> PendingKicks; // PlayerId -> Reason
	TMap<FString, FLobbyPlayerInfo> PendingKickInfo; // PlayerId -> PlayerInfo (cached before removal)

	// UTILITY FUNCTIONS
	void PrintDebugMessage(const FString& Message, bool isError);

	// Lobby Utilities
	// ------------------------
	FString HashPassword(const FString& Password) const;
	bool ValidatePassword(const FString& Password, const FString& StoredHash) const;
	FLobbyInfo CreateLobbyInfoFromSession() const;
	FLobbyInfo ConvertSearchResultToLobbyInfo(const FOnlineSessionSearchResult& SearchResult) const;
};
