// LobbyGameMode.cpp

#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "LobbyGameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

ALobbyGameMode::ALobbyGameMode()
{
	// Configure lobby-specific classes
	PlayerStateClass = ALobbyPlayerState::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Bind to ready state changes from GameState
	if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
	{
		LobbyGameState->OnPlayerReadyStateChanged.AddDynamic(this, &ALobbyGameMode::OnPlayerReadyStateChanged);
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 NumberOfPlayers = GameState->PlayerArray.Num();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Blue,
				FString::Printf(TEXT("Players in lobby: %d"), NumberOfPlayers)
			);

			APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>();
			if (PS)
			{
				FString PlayerName = PS->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(
					-1,
					60.f,
					FColor::Cyan,
					FString::Printf(TEXT("%s has joined the lobby!"), *PlayerName)
				);
			}
		}
	}

	// New player joining means not all players are ready anymore
	if (bWasAllReady)
	{
		bWasAllReady = false;
		OnNotAllPlayersReady.Broadcast();
	}
}

void ALobbyGameMode::Logout(AController* ExitingPlayer)
{
	APlayerState* PS = ExitingPlayer->GetPlayerState<APlayerState>();
	if (PS && GEngine)
	{
		FString PlayerName = PS->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(
			-1,
			60.f,
			FColor::Orange,
			FString::Printf(TEXT("%s has left the lobby!"), *PlayerName)
		);
	}

	Super::Logout(ExitingPlayer);

	if (GameState && GEngine)
	{
		// PlayerArray count is already updated after Super::Logout
		int32 NumberOfPlayers = GameState->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(
			1,
			60.f,
			FColor::Blue,
			FString::Printf(TEXT("Players in lobby: %d"), NumberOfPlayers)
		);
	}

	// Check if remaining players are all ready
	if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
	{
		bool bAllReady = LobbyGameState->AreAllPlayersReady();
		if (bAllReady && !bWasAllReady)
		{
			bWasAllReady = true;
			OnAllPlayersReady.Broadcast();
		}
	}
}

void ALobbyGameMode::OnPlayerReadyStateChanged(ALobbyPlayerState* PlayerState, bool bIsReady)
{
	// Debug message
	FString PlayerName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Unknown");
	FString ReadyStatus = bIsReady ? TEXT("READY") : TEXT("NOT READY");
	FColor MessageColor = bIsReady ? FColor::Green : FColor::Yellow;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, MessageColor,
		                                 FString::Printf(TEXT("%s is %s"), *PlayerName, *ReadyStatus));
	}

	CheckReadyState();
}

void ALobbyGameMode::CheckReadyState()
{
	ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();
	if (!LobbyGS)
	{
		return;
	}

	bool bAllReady = LobbyGS->AreAllPlayersReady();

	// Detect transition: was NOT all ready, now IS
	if (bAllReady && !bWasAllReady)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			                                 TEXT("All players are READY!"));
		}
		OnAllPlayersReady.Broadcast();
	}

	// Detect transition: WAS all ready, now NOT
	else if (!bAllReady && bWasAllReady)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
			                                 TEXT("Waiting for all players..."));
		}
		OnNotAllPlayersReady.Broadcast();
	}

	bWasAllReady = bAllReady;
}

bool ALobbyGameMode::CanStartGame() const
{
	if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
	{
		return LobbyGameState->AreAllPlayersReady() && LobbyGameState->GetTotalPlayerCount() > 0;
	}
	return false;
}

bool ALobbyGameMode::StartGame(const FString& GameLevelPath)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartGame called on client - only server can start the game"));
		return false;
	}

	if (!CanStartGame())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start game - not all players are ready"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				TEXT("Cannot start: Not all players are ready or not enough players!")
			);
		}
		return false;
	}

	if (GameLevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("StartGame called with empty level path"));
		return false;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Green,
				FString::Printf(TEXT("Starting game: %s"), *GameLevelPath)
			);
		}

		// Server travel takes all connected clients to the new level
		bool bSuccess = World->ServerTravel(GameLevelPath);
		return bSuccess;
	}

	return false;
}
