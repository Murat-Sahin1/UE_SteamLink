// LobbyPlayerEntryWidget.cpp

#include "LobbyPlayerEntryWidget.h"
#include "LobbyPlayerState.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "GameFramework/GameStateBase.h"

void ULobbyPlayerEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULobbyPlayerEntryWidget::NativeDestruct()
{
	// Unbind from player state if we have one
	if (PlayerState)
	{
		PlayerState->OnReadyStateChanged.RemoveDynamic(this, &ULobbyPlayerEntryWidget::OnPlayerReadyStateChanged);
	}

	Super::NativeDestruct();
}

void ULobbyPlayerEntryWidget::SetPlayerState(ALobbyPlayerState* InPlayerState)
{
	// Unbind from old player state
	if (PlayerState)
	{
		PlayerState->OnReadyStateChanged.RemoveDynamic(this, &ULobbyPlayerEntryWidget::OnPlayerReadyStateChanged);
	}

	PlayerState = InPlayerState;

	if (PlayerState)
	{
		// Bind to ready state changes
		PlayerState->OnReadyStateChanged.AddDynamic(this, &ULobbyPlayerEntryWidget::OnPlayerReadyStateChanged);

		// Check if this is the local player
		if (APlayerController* PC = GetOwningPlayer())
		{
			bIsLocalPlayer = (PlayerState == PC->GetPlayerState<ALobbyPlayerState>());
		}
	}

	UpdateDisplay();
}

void ULobbyPlayerEntryWidget::RefreshDisplay()
{
	UpdateDisplay();
}

void ULobbyPlayerEntryWidget::OnPlayerReadyStateChanged(ALobbyPlayerState* ChangedPlayerState, bool bIsReady)
{
	// Only update if it's our player state
	if (ChangedPlayerState == PlayerState)
	{
		UpdateDisplay();
	}
}

void ULobbyPlayerEntryWidget::UpdateDisplay()
{
	if (!PlayerState)
	{
		// Clear display if no player state
		if (PlayerNameText)
		{
			PlayerNameText->SetText(FText::FromString(TEXT("---")));
		}
		if (ReadyStatusText)
		{
			ReadyStatusText->SetText(FText::FromString(TEXT("")));
		}
		if (HostIcon)
		{
			HostIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (ReadyIcon)
		{
			ReadyIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	// Update player name
	if (PlayerNameText)
	{
		FString DisplayName = PlayerState->GetPlayerName();
		if (bIsLocalPlayer)
		{
			DisplayName += TEXT(" (You)");
		}
		PlayerNameText->SetText(FText::FromString(DisplayName));
	}

	// Update ready status
	bool bIsReady = PlayerState->IsReady();
	if (ReadyStatusText)
	{
		if (bIsReady)
		{
			ReadyStatusText->SetText(FText::FromString(TEXT("Ready")));
			ReadyStatusText->SetColorAndOpacity(ReadyTextColor);
		}
		else
		{
			ReadyStatusText->SetText(FText::FromString(TEXT("Not Ready")));
			ReadyStatusText->SetColorAndOpacity(NotReadyTextColor);
		}
	}

	// Update ready icon visibility
	if (ReadyIcon)
	{
		ReadyIcon->SetVisibility(bIsReady ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Update host icon visibility
	if (HostIcon)
	{
		bool bIsHost = false;
		
		if (UWorld* World = GetWorld())
		{
			// In a listen server, the first player (index 0) is typically the host
			if (AGameStateBase* GameState = World->GetGameState())
			{
				if (GameState->PlayerArray.Num() > 0 && GameState->PlayerArray[0] == PlayerState)
				{
					bIsHost = true;
				}
			}
		}

		HostIcon->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Update background color based on state
	if (BackgroundBorder)
	{
		FLinearColor BgColor;
		if (bIsLocalPlayer)
		{
			BgColor = LocalPlayerColor;
		}
		else if (bIsReady)
		{
			BgColor = ReadyColor;
		}
		else
		{
			BgColor = NotReadyColor;
		}
		BackgroundBorder->SetBrushColor(BgColor);
	}
}
