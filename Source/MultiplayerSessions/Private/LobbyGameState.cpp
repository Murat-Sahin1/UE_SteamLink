// LobbyGameState.cpp

#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "GameFramework/PlayerState.h"

ALobbyGameState::ALobbyGameState()
{
}

bool ALobbyGameState::AreAllPlayersReady() const
{
	if (PlayerArray.Num() == 0)
	{
		return false;
	}

	for (APlayerState* PS : PlayerArray)
	{
		ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PS);
		if (LobbyPlayerState && !LobbyPlayerState->IsReady())
		{
			return false;
		}
	}

	return true;
}

int32 ALobbyGameState::GetReadyPlayerCount() const
{
	int32 ReadyCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PS);
		if (LobbyPlayerState && LobbyPlayerState->IsReady())
		{
			ReadyCount++;
		}
	}

	return ReadyCount;
}

int32 ALobbyGameState::GetTotalPlayerCount() const
{
	return PlayerArray.Num();
}

TArray<ALobbyPlayerState*> ALobbyGameState::GetLobbyPlayerStates() const
{
	TArray<ALobbyPlayerState*> LobbyPlayers;

	for (APlayerState* PS : PlayerArray)
	{
		if (ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PS))
		{
			LobbyPlayers.Add(LobbyPlayerState);
		}
	}

	return LobbyPlayers;
}

void ALobbyGameState::BroadcastReadyStateChanged(APlayerState* PS, bool bIsReady)
{
	if (ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PS))
	{
		OnPlayerReadyStateChanged.Broadcast(LobbyPlayerState, bIsReady);
	}
}
