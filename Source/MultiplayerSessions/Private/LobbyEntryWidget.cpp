// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"

void ULobbyEntryWidget::SetLobbyInfo(const FLobbyInfo& InLobbyInfo)
{
	LobbyInfo = InLobbyInfo;
	UpdateDisplay();
}

void ULobbyEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button callbacks
	if (EntryButton)
	{
		EntryButton->OnClicked.AddDynamic(this, &ThisClass::OnEntryButtonClicked);
		EntryButton->OnHovered.AddDynamic(this, &ThisClass::OnEntryHovered);
		EntryButton->OnUnhovered.AddDynamic(this, &ThisClass::OnEntryUnhovered);
	}

	// Set initial background color
	if (BackgroundBorder)
	{
		FLinearColor InitialColor = LobbyInfo.bIsPublic ? NormalColor : PrivateLobbyColor;
		BackgroundBorder->SetBrushColor(InitialColor);
	}

	// Update display with current data
	UpdateDisplay();
}

void ULobbyEntryWidget::NativeDestruct()
{
	// Unbind button callbacks
	if (EntryButton)
	{
		EntryButton->OnClicked.RemoveDynamic(this, &ThisClass::OnEntryButtonClicked);
		EntryButton->OnHovered.RemoveDynamic(this, &ThisClass::OnEntryHovered);
		EntryButton->OnUnhovered.RemoveDynamic(this, &ThisClass::OnEntryUnhovered);
	}

	Super::NativeDestruct();
}

void ULobbyEntryWidget::OnEntryButtonClicked()
{
	// Broadcast to parent that this entry was clicked
	OnEntryClicked.Broadcast(LobbyInfo);
}

void ULobbyEntryWidget::OnEntryHovered()
{
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(HoveredColor);
	}
}

void ULobbyEntryWidget::OnEntryUnhovered()
{
	if (BackgroundBorder)
	{
		FLinearColor NormalColorToUse = LobbyInfo.bIsPublic ? NormalColor : PrivateLobbyColor;
		BackgroundBorder->SetBrushColor(NormalColorToUse);
	}
}

void ULobbyEntryWidget::UpdateDisplay()
{
	// Update host name
	if (HostNameText)
	{
		HostNameText->SetText(FText::FromString(LobbyInfo.HostName));
	}

	// Update player count (e.g., "3/5")
	if (PlayerCountText)
	{
		FString PlayerCountString = FString::Printf(TEXT("%d/%d"),
		                                            LobbyInfo.CurrentPlayerCount,
		                                            LobbyInfo.MaxPlayerCount);
		PlayerCountText->SetText(FText::FromString(PlayerCountString));
	}

	// Update ping (e.g., "45 ms")
	if (PingText)
	{
		if (LobbyInfo.PingInMs >= 0)
		{
			FString PingString = FString::Printf(TEXT("%dms"), LobbyInfo.PingInMs);
			PingText->SetText(FText::FromString(PingString));
		}
		else
		{
			PingText->SetText(FText::FromString(TEXT("N/A")));
		}
	}

	// Show/hide lock icon based on visibility
	if (LockIcon)
	{
		LockIcon->SetVisibility(LobbyInfo.bIsPublic ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	// Update background color based on public/private
	if (BackgroundBorder)
	{
		FLinearColor ColorToUse = LobbyInfo.bIsPublic ? NormalColor : PrivateLobbyColor;
		BackgroundBorder->SetBrushColor(ColorToUse);
	}
}
