// LobbyGameMode.h
// Game mode for lobby level

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

class ALobbyGameState;
class ALobbyPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPlayersReady);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotAllPlayersReady);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStarted,
                                            bool, bWasSuccessful);

/**
 * Game mode for the lobby level.
 * Handles player connections and ready-up system.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitingPlayer) override;

	/**
	 * Start the game and travel to the game level.
	 * Only works if called on server and all players are ready.
	 * @param GameLevelPath - The path to the game level (e.g., "/Game/Maps/GameLevel?listen")
	 * @return True if game start was initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool StartGame(const FString& GameLevelPath);

	/** Check if the game can be started (all players ready) */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool CanStartGame() const;

	/** Broadcast when all players become ready */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnAllPlayersReady OnAllPlayersReady;

	/** Broadcast when not all players are ready (someone unreadied) */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnNotAllPlayersReady OnNotAllPlayersReady;

protected:
	/** Called when any player's ready state changes */
	UFUNCTION()
	void OnPlayerReadyStateChanged(ALobbyPlayerState* PlayerState, bool bIsReady);

	virtual void BeginPlay() override;

private:
	/** Track if all players were previously ready */
	bool bWasAllReady = false;

	void CheckReadyState();
};
