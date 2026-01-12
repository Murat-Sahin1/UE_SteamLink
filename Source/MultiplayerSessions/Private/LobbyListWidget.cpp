// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyListWidget.h"
#include "MultiplayerSessionsSubsystem.h"
#include "LobbyEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void ULobbyListWidget::Setup(UMultiplayerSessionsSubsystem* Subsystem)
{
	SessionsSubsystem = Subsystem;

	if (SessionsSubsystem)
	{
		SessionsSubsystem->MultiplayerOnLobbyListUpdated.AddDynamic(this, &ThisClass::OnLobbyListUpdated);
	}
}

void ULobbyListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button callbacks
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &ThisClass::OnRefreshButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::OnBackButtonClicked);
	}

	// Initialize UI state
	SetLoadingState(false);
	UpdateStatusText(0);

	if (EmptyStateText)
	{
		EmptyStateText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULobbyListWidget::NativeDestruct()
{
	// Unbind subsystem delegate
	if (SessionsSubsystem)
	{
		SessionsSubsystem->MultiplayerOnLobbyListUpdated.RemoveDynamic(this, &ThisClass::OnLobbyListUpdated);
	}

	// Unbind button callbacks
	if (RefreshButton)
	{
		RefreshButton->OnClicked.RemoveDynamic(this, &ThisClass::OnRefreshButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &ThisClass::OnBackButtonClicked);
	}

	// Clean up lobby entry widgets
	ClearLobbyList();

	Super::NativeDestruct();
}

void ULobbyListWidget::RefreshLobbyList()
{
	if (!SessionsSubsystem)
	{
		return;
	}

	SetLoadingState(true);
	ClearLobbyList();

	SessionsSubsystem->FindLobbies(MaxSearchResults);
}

void ULobbyListWidget::OnRefreshButtonClicked()
{
	RefreshLobbyList();
}

void ULobbyListWidget::OnBackButtonClicked()
{
	OnBackButtonPressed.Broadcast();
}

void ULobbyListWidget::OnLobbyListUpdated(const TArray<FLobbyInfo>& Lobbies, bool bWasSuccessful)
{
	SetLoadingState(false);

	if (bWasSuccessful)
	{
		PopulateLobbyList(Lobbies);
		UpdateStatusText(Lobbies.Num());

		// Show/hide empty state
		if (EmptyStateText)
		{
			EmptyStateText->SetVisibility(Lobbies.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}
	else
	{
		// Show error state
		ClearLobbyList();
		UpdateStatusText(0);
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("Failed to find lobbies")));
		}
	}
}

void ULobbyListWidget::OnLobbyEntryClicked(const FLobbyInfo& LobbyInfo)
{
	// Boradcast to parent (Menu) that a lobby was selected
	OnLobbySelected.Broadcast(LobbyInfo);
}

void ULobbyListWidget::PopulateLobbyList(const TArray<FLobbyInfo>& Lobbies)
{
	ClearLobbyList();

	if (!LobbyScrollBox || !LobbyEntryWidgetClass)
	{
		return;
	}

	for (const FLobbyInfo& Lobby : Lobbies)
	{
		// Create lobby entry widget
		ULobbyEntryWidget* EntryWidget = CreateWidget<ULobbyEntryWidget>(this, LobbyEntryWidgetClass);
		if (EntryWidget)
		{
			// Set lobby data
			EntryWidget->SetLobbyInfo(Lobby);

			// Bind click callback
			EntryWidget->OnEntryClicked.AddDynamic(this, &ThisClass::OnLobbyEntryClicked);

			// Add to scroll box
			LobbyScrollBox->AddChild(EntryWidget);

			// Store reference
			LobbyEntryWidgets.Add(EntryWidget);
		}
	}
}

void ULobbyListWidget::ClearLobbyList()
{
	if (LobbyScrollBox)
	{
		LobbyScrollBox->ClearChildren();
	}

	// Unbind delegates from old widgets
	for (ULobbyEntryWidget* EntryWidget : LobbyEntryWidgets)
	{
		if (EntryWidget)
		{
			EntryWidget->OnEntryClicked.RemoveDynamic(this, &ThisClass::OnLobbyEntryClicked);
		}
	}

	LobbyEntryWidgets.Empty();
}

void ULobbyListWidget::SetLoadingState(bool bIsLoading)
{
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bIsLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(!bIsLoading);
	}

	if (LobbyScrollBox)
	{
		LobbyScrollBox->SetVisibility(bIsLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void ULobbyListWidget::UpdateStatusText(int32 LobbyCount)
{
	if (StatusText)
	{
		if (LobbyCount == 0)
		{
			StatusText->SetText(FText::FromString(TEXT("No lobbies found")));
		}
		else if (LobbyCount == 1)
		{
			StatusText->SetText(FText::FromString(TEXT("Found 1 lobby")));
		}
		else
		{
			StatusText->SetText(FText::FromString(FString::Printf(TEXT("Found %d lobbies"), LobbyCount)));
		}
	}
}
