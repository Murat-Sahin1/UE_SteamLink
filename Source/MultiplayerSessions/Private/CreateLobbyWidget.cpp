// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateLobbyWidget.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"

void UCreateLobbyWidget::Setup(UMultiplayerSessionsSubsystem* Subsystem)
{
	SessionsSubsystem = Subsystem;

	if (SessionsSubsystem)
	{
		SessionsSubsystem->MultiplayerOnLobbyCreated.AddDynamic(this, &ThisClass::OnLobbyCreated);
	}
}

void UCreateLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button callbacks
	if (CreateButton)
	{
		CreateButton->OnClicked.AddDynamic(this, &ThisClass::OnCreateButtonClicked);
	}
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &ThisClass::OnCancelButtonClicked);
	}
	if (PublicCheckBox)
	{
		PublicCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnPublicCheckBoxChanged);

		// Set default to public
		PublicCheckBox->SetIsChecked(true);
	}

	// Set default values
	if (MaxPlayersSpinBox)
	{
		MaxPlayersSpinBox->SetMinValue(2);
		MaxPlayersSpinBox->SetMinSliderValue(2);
		MaxPlayersSpinBox->SetMaxValue(16);
		MaxPlayersSpinBox->SetMaxSliderValue(16);
		MaxPlayersSpinBox->SetValue(4);
	}

	// Initialize UI State
	UpdatePasswordVisibility();
	ClearError();
	SetLoadingState(false);
}

void UCreateLobbyWidget::NativeDestruct()
{
	// Unbind subsystem delegate
	if (SessionsSubsystem)
	{
		SessionsSubsystem->MultiplayerOnLobbyCreated.RemoveDynamic(this, &ThisClass::OnLobbyCreated);
	}

	// Unbind button calbacks
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveDynamic(this, &ThisClass::OnCreateButtonClicked);
	}
	if (CancelButton)
	{
		CancelButton->OnClicked.RemoveDynamic(this, &ThisClass::OnCancelButtonClicked);
	}
	if (PublicCheckBox)
	{
		PublicCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::OnPublicCheckBoxChanged);
	}

	Super::NativeDestruct();
}

void UCreateLobbyWidget::OnCreateButtonClicked()
{
	if (bIsCreatingLobby)
	{
		return;
	}

	if (!ValidateInput())
	{
		return;
	}

	if (!SessionsSubsystem)
	{
		ShowError(TEXT("Session system not available."));
		return;
	}

	ClearError();
	SetLoadingState(true);

	// Build lobby settings from UI
	FLobbySettings Settings;
	Settings.MaxPlayers = FMath::RoundToInt(MaxPlayersSpinBox->GetValue());
	Settings.bIsPublic = PublicCheckBox->IsChecked();

	if (!Settings.bIsPublic && PasswordInput)
	{
		Settings.Password = PasswordInput->GetText().ToString();
	}

	// Create the lobby
	SessionsSubsystem->CreateLobby(Settings);
}

void UCreateLobbyWidget::OnCancelButtonClicked()
{
	if (bIsCreatingLobby)
	{
		return;
	}

	// Notify parent that creation was cancelled
	OnLobbyCreationComplete.Broadcast(false);
}

void UCreateLobbyWidget::OnPublicCheckBoxChanged(bool bIsChecked)
{
	UpdatePasswordVisibility();
	ClearError();
}

void UCreateLobbyWidget::OnLobbyCreated(bool bWasSuccessful, const FLobbyInfo& LobbyInfo)
{
	SetLoadingState(false);

	if (bWasSuccessful)
	{
		// Notify parent of success
		OnLobbyCreationComplete.Broadcast(true);
	}
	else
	{
		ShowError(TEXT("Failed to create lobby. Please try again."));
	}
}

bool UCreateLobbyWidget::ValidateInput()
{
	// Check if private lobby has a password
	if (PublicCheckBox && !PublicCheckBox->IsChecked())
	{
		if (PasswordInput)
		{
			FString Password = PasswordInput->GetText().ToString();
			if (Password.IsEmpty())
			{
				ShowError(TEXT("Password is required for private lobbies."));
				return false;
			}

			if (Password.Len() < 4)
			{
				ShowError(TEXT("Password must be at least 4 characters"));
				return false;
			}
		}
	}

	// Validate player count
	if (MaxPlayersSpinBox)
	{
		int32 MaxPlayers = FMath::RoundToInt(MaxPlayersSpinBox->GetValue());
		if (MaxPlayers < 2 || MaxPlayers > 16)
		{
			ShowError(TEXT("Max players must be between 2 and 16"));
			return false;
		}
	}

	return true;
}

void UCreateLobbyWidget::UpdatePasswordVisibility()
{
	if (PasswordContainer)
	{
		bool bIsPublic = PublicCheckBox ? PublicCheckBox->IsChecked() : false;
		PasswordContainer->SetVisibility(bIsPublic ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	// Clear password when switching to public
	if (PublicCheckBox && PublicCheckBox->IsChecked() && PasswordInput)
	{
		PasswordInput->SetText(FText::GetEmpty());
	}
}

void UCreateLobbyWidget::SetLoadingState(bool bIsLoading)
{
	bIsCreatingLobby = bIsLoading;

	if (CreateButton)
	{
		CreateButton->SetIsEnabled(!bIsLoading);
	}

	if (CancelButton)
	{
		CancelButton->SetIsEnabled(!bIsLoading);
	}

	if (MaxPlayersSpinBox)
	{
		MaxPlayersSpinBox->SetIsEnabled(!bIsLoading);
	}

	if (PublicCheckBox)
	{
		PublicCheckBox->SetIsEnabled(!bIsLoading);
	}

	if (PasswordInput)
	{
		PasswordInput->SetIsEnabled(!bIsLoading);
	}

	if (LobbyNameInput)
	{
		LobbyNameInput->SetIsEnabled(!bIsLoading);
	}
}

void UCreateLobbyWidget::ShowError(const FString& Message)
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(Message));
		ErrorText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCreateLobbyWidget::ClearError()
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Hidden);
	}
}
