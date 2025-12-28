#pragma once

#include "CoreMinimal.h"
#include "MultiplayerSessionsTypes.generated.h"


UENUM(BlueprintType)
enum class ELobbyJoinResult : uint8
{
	Success, LobbyFull, WrongPassword,
	LobbyNotFound, ConnectionFailed, UnknownError
};

UENUM(BlueprintType)
enum class ELobbyLeaveReason: uint8
{
	Left, Kicked, Disconnected,
	SessionDestroyed, HostMigration, Unknown
};

USTRUCT(BlueprintType)
struct MULTIPLAYERSESSIONS_API FLobbyInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FString LobbyId;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FString HostName;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	int32 CurrentPlayerCount;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	int32 MaxPlayerCount;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	bool bIsPublic;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	int32 PingInMs;

	FLobbyInfo() :
		CurrentPlayerCount(0),
		MaxPlayerCount(0),
		bIsPublic(true),
		PingInMs(-1)
	{
	}
};

USTRUCT(BlueprintType)
struct MULTIPLAYERSESSIONS_API FLobbySettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	bool bIsPublic;

	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	FString Password;

	FLobbySettings() :
		MaxPlayers(10),
		bIsPublic(true)
	{
	}
};

USTRUCT(BlueprintType)
struct MULTIPLAYERSESSIONS_API FLobbyPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	bool bIsHost;

	FLobbyPlayerInfo() :
		bIsHost(false)
	{
	}
};
