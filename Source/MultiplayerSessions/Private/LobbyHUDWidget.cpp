// LobbyHUDWidget.cpp

#include "LobbyHUDWidget.h"
#include "LobbyGameState.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

void ULobbyHUDWidget::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeUIOnly;
			InputModeUIOnly.SetWidgetToFocus(TakeWidget());
			InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PlayerController->SetInputMode(InputModeUIOnly);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void ULobbyHUDWidget::MenuTearDown()
{
	UWorld* World = GetWorld();

	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();

		if (PlayerController)
		{
			FInputModeGameOnly InputModeGameOnly;
			PlayerController->SetInputMode(InputModeGameOnly);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	RemoveFromParent();
}

void ULobbyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnReadyButtonClicked);
	}

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnStartGameButtonClicked);
	}

	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnLeaveButtonClicked);
	}

	// Auto-initialize if not done manually
	if (!bIsInitialized)
	{
		InitializeLobbyHUD();
	}
}

void ULobbyHUDWidget::NativeDestruct()
{
	UnbindFromGameState();

	// Clear player entries
	for (ULobbyPlayerEntryWidget* Entry : PlayerEntryWidgets)
	{
		if (Entry)
		{
			Entry->RemoveFromParent();
		}
	}
	PlayerEntryWidgets.Empty();

	MenuTearDown();
	Super::NativeDestruct();
}

void ULobbyHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Periodic refresh to catch late joiners and state changes
	RefreshTimer += InDeltaTime;
	if (RefreshTimer >= RefreshInterval)
	{
		RefreshTimer = 0.0f;

		// Re-bind if GameState changed (e.g., seamless travel)
		if (!CachedGameState || !CachedGameState->IsValidLowLevel())
		{
			BindToGameState();
		}

		// Check if player count changed
		if (CachedGameState)
		{
			int32 CurrentCount = CachedGameState->GetTotalPlayerCount();
			if (CurrentCount != PlayerEntryWidgets.Num())
			{
				RebuildPlayerList();
			}
		}

		UpdateStatusText();
		UpdateStartButton();
	}
}

void ULobbyHUDWidget::InitializeLobbyHUD()
{
	if (bIsInitialized)
	{
		return;
	}

	bIsInitialized = true;
	BindToGameState();
	RebuildPlayerList();
	UpdateReadyButton();
	UpdateStartButton();
	UpdateStatusText();
}

void ULobbyHUDWidget::RefreshPlayerList()
{
	RebuildPlayerList();
	UpdateStatusText();
	UpdateStartButton();
}

bool ULobbyHUDWidget::IsLocalPlayerHost() const
{
	// In a listen server, check if we have authority
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Check if this is the server/host
	return World->GetNetMode() == NM_ListenServer || World->GetNetMode() == NM_DedicatedServer;
}

void ULobbyHUDWidget::BindToGameState()
{
	UnbindFromGameState();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	CachedGameState = World->GetGameState<ALobbyGameState>();
	if (CachedGameState)
	{
		CachedGameState->OnPlayerReadyStateChanged.AddDynamic(this, &ULobbyHUDWidget::OnPlayerReadyStateChanged);
	}

	// Also bind to GameMode events if we're the server
	if (IsLocalPlayerHost())
	{
		if (ALobbyGameMode* GameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(World)))
		{
			GameMode->OnAllPlayersReady.AddDynamic(this, &ULobbyHUDWidget::OnAllPlayersReady);
			GameMode->OnNotAllPlayersReady.AddDynamic(this, &ULobbyHUDWidget::OnNotAllPlayersReady);
		}
	}
}

void ULobbyHUDWidget::UnbindFromGameState()
{
	if (CachedGameState)
	{
		CachedGameState->OnPlayerReadyStateChanged.RemoveDynamic(this, &ULobbyHUDWidget::OnPlayerReadyStateChanged);
	}

	if (IsLocalPlayerHost())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			if (ALobbyGameMode* GameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(World)))
			{
				GameMode->OnAllPlayersReady.RemoveDynamic(this, &ULobbyHUDWidget::OnAllPlayersReady);
				GameMode->OnNotAllPlayersReady.RemoveDynamic(this, &ULobbyHUDWidget::OnNotAllPlayersReady);
			}
		}
	}

	CachedGameState = nullptr;
}

void ULobbyHUDWidget::RebuildPlayerList()
{
	// Clear existing entries
	if (PlayerListScrollBox)
	{
		PlayerListScrollBox->ClearChildren();
	}

	for (ULobbyPlayerEntryWidget* Entry : PlayerEntryWidgets)
	{
		if (Entry)
		{
			Entry->RemoveFromParent();
		}
	}
	PlayerEntryWidgets.Empty();

	// Get all player states
	if (!CachedGameState)
	{
		return;
	}

	TArray<ALobbyPlayerState*> PlayerStates = CachedGameState->GetLobbyPlayerStates();

	// Check if we have a valid entry widget class
	if (!PlayerEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyHUDWidget: PlayerEntryWidgetClass is not set!"));
		return;
	}

	// Create entry widgets for each player
	for (ALobbyPlayerState* PS : PlayerStates)
	{
		if (!PS)
		{
			continue;
		}

		ULobbyPlayerEntryWidget* EntryWidget = CreateWidget<ULobbyPlayerEntryWidget>(
			GetOwningPlayer(), PlayerEntryWidgetClass);
		if (EntryWidget)
		{
			EntryWidget->SetPlayerState(PS);
			PlayerEntryWidgets.Add(EntryWidget);

			if (PlayerListScrollBox)
			{
				PlayerListScrollBox->AddChild(EntryWidget);
			}
		}
	}

	// Update player count text
	if (PlayerCountText && CachedGameState)
	{
		int32 TotalPlayers = CachedGameState->GetTotalPlayerCount();
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d Players"), TotalPlayers)));
	}
}

void ULobbyHUDWidget::UpdateReadyButton()
{
	ALobbyPlayerState* LocalPS = GetLocalPlayerState();
	if (!LocalPS || !ReadyButtonText)
	{
		return;
	}

	bool bIsReady = LocalPS->IsReady();
	if (bIsReady)
	{
		ReadyButtonText->SetText(FText::FromString(TEXT("Cancel Ready")));
	}
	else
	{
		ReadyButtonText->SetText(FText::FromString(TEXT("Ready")));
	}
}

void ULobbyHUDWidget::UpdateStartButton()
{
	if (!StartGameButton)
	{
		return;
	}

	bool bIsHost = IsLocalPlayerHost();

	// Only host can see the start button
	StartGameButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bIsHost && CachedGameState)
	{
		bool bCanStart = CachedGameState->AreAllPlayersReady() && CachedGameState->GetTotalPlayerCount() > 0;
		StartGameButton->SetIsEnabled(bCanStart);

		if (StartGameButtonText)
		{
			if (bCanStart)
			{
				StartGameButtonText->SetText(FText::FromString(TEXT("Start Game")));
			}
			else
			{
				StartGameButtonText->SetText(FText::FromString(TEXT("Waiting...")));
			}
		}
	}
}

void ULobbyHUDWidget::UpdateStatusText()
{
	if (!StatusText || !CachedGameState)
	{
		return;
	}

	int32 ReadyCount = CachedGameState->GetReadyPlayerCount();
	int32 TotalCount = CachedGameState->GetTotalPlayerCount();
	bool bAllReady = CachedGameState->AreAllPlayersReady() && TotalCount > 0;

	if (bAllReady)
	{
		StatusText->SetText(FText::FromString(TEXT("All players ready!")));
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
	}
	else
	{
		StatusText->SetText(
			FText::FromString(FString::Printf(TEXT("Waiting for players... (%d/%d ready)"), ReadyCount, TotalCount)));
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
	}
}

ALobbyPlayerState* ULobbyHUDWidget::GetLocalPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		return PC->GetPlayerState<ALobbyPlayerState>();
	}
	return nullptr;
}

void ULobbyHUDWidget::OnReadyButtonClicked()
{
	ALobbyPlayerState* LocalPS = GetLocalPlayerState();
	if (LocalPS)
	{
		LocalPS->ToggleReadyState();
		UpdateReadyButton();
	}
}

void ULobbyHUDWidget::OnStartGameButtonClicked()
{
	if (!IsLocalPlayerHost())
	{
		UE_LOG(LogTemp, Warning, TEXT("Only the host can start the game!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ALobbyGameMode* GameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(World));
	if (GameMode)
	{
		bool bStarted = GameMode->StartGame(GameLevelPath);

		if (!bStarted)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to start game"));
		}
		else
		{
			MenuTearDown();
		}
	}
}

void ULobbyHUDWidget::OnLeaveButtonClicked()
{
	// Return to main menu or disconnect
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// Simple approach: travel to main menu level
		// You may want to customize this based on your game's flow
		UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Maps/Lvl_MainMenu")));
	}
}

void ULobbyHUDWidget::OnPlayerReadyStateChanged(ALobbyPlayerState* PlayerState, bool bIsReady)
{
	// Update the ready button if it's the local player
	ALobbyPlayerState* LocalPS = GetLocalPlayerState();
	if (PlayerState == LocalPS)
	{
		UpdateReadyButton();
	}

	// Refresh displays
	UpdateStatusText();
	UpdateStartButton();
}

void ULobbyHUDWidget::OnAllPlayersReady()
{
	UpdateStatusText();
	UpdateStartButton();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
		                                 TEXT("All players are ready! Host can start the game."));
	}
}

void ULobbyHUDWidget::OnNotAllPlayersReady()
{
	UpdateStatusText();
	UpdateStartButton();
}
