// LobbyPlayerState.cpp

#include "LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "LobbyGameState.h"
#include "GameFramework/GameStateBase.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	// PlayerState is already set to replicate by default
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
}

void ALobbyPlayerState::SetReadyState(bool bNewReady)
{
	if (HasAuthority())
	{
		// Server can set directly
		if (bIsReady != bNewReady)
		{
			bIsReady = bNewReady;
			// OnRep won't fire on server, so broadcast manually
			OnRep_bIsReady();
		}
	}
	else
	{
		// Client must request via RPC
		Server_SetReadyState(bNewReady);
	}
}

void ALobbyPlayerState::ToggleReadyState()
{
	SetReadyState(!bIsReady);
}

void ALobbyPlayerState::Server_SetReadyState_Implementation(bool bNewReady)
{
	if (bIsReady != bNewReady)
	{
		bIsReady = bNewReady;
		// OnRep won't fire on server, so broadcast manually
		OnRep_bIsReady();
	}
}

void ALobbyPlayerState::OnRep_bIsReady()
{
	// Broadcast local delegate
	OnReadyStateChanged.Broadcast(this, bIsReady);

	// Also notify the GameState so it can broadcast to listeners
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			LobbyGameState->BroadcastReadyStateChanged(this, bIsReady);
		}
	}

	// Debug output
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			bIsReady ? FColor::Green : FColor::Yellow,
			FString::Printf(TEXT("%s is %s"), *GetPlayerName(), bIsReady ? TEXT("READY") : TEXT("NOT READY"))
		);
	}
}
