// LobbyPlayerState.h
// Player state for lobby with ready-up functionality

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReadyStateChanged,
	ALobbyPlayerState*, PlayerState,
	bool, bIsReady);

/**
 * Player state for the lobby level with ready-up functionality.
 * Ready state is replicated to all clients.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ALobbyPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Set the ready state. Clients call this to request a state change,
	 * which is then sent to the server for validation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReadyState(bool bNewReady);

	/** Toggle the current ready state */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ToggleReadyState();

	/** Check if this player is ready */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsReady() const { return bIsReady; }

	/** Delegate broadcast when ready state changes (fires on all clients) */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnReadyStateChanged OnReadyStateChanged;

protected:
	/** Server RPC to set ready state */
	UFUNCTION(Server, Reliable)
	void Server_SetReadyState(bool bNewReady);

	/** Called when bIsReady is replicated */
	UFUNCTION()
	void OnRep_bIsReady();

private:
	/** Whether this player is ready to start the game */
	UPROPERTY(ReplicatedUsing = OnRep_bIsReady)
	bool bIsReady = false;
};
