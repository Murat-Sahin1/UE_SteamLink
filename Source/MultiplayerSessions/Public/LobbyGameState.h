// LobbyGameState.h
// Game state for lobby level

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

class ALobbyPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyPlayerReadyStateChanged,
	ALobbyPlayerState*, PlayerState,
	bool, bIsReady);

/**
 * Game state for the lobby level.
 * Tracks all players' ready states and provides query methods.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALobbyGameState();

	/**
	 * Check if all connected players are ready.
	 * Returns false if no players are connected.
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool AreAllPlayersReady() const;

	/** Get the number of players who are ready */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	int32 GetReadyPlayerCount() const;

	/** Get total player count in the lobby */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	int32 GetTotalPlayerCount() const;

	/** Get all lobby player states */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	TArray<ALobbyPlayerState*> GetLobbyPlayerStates() const;

	/**
	 * Broadcast when any player's ready state changes.
	 * UI can bind to this to update player list displays.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerReadyStateChanged OnPlayerReadyStateChanged;

	/** Called by LobbyPlayerState when ready state changes */
	void BroadcastReadyStateChanged(APlayerState* PlayerState, bool bIsReady);
};
