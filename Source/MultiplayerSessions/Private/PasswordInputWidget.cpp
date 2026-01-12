// Fill out your copyright notice in the Description page of Project Settings.


#include "PasswordInputWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UPasswordInputWidget::ShowForLobby(const FLobbyInfo& InLobbyInfo)
{
	TargetLobbyInfo = InLobbyInfo;

	// Update lobby name text
	if (LobbyNameText)
	{
		FString LobbyDisplayName = FString::Printf(TEXT("Join %s's Lobby"), *TargetLobbyInfo.HostName);
		LobbyNameText->SetText(FText::FromString(LobbyDisplayName));
	}

	// Reset password field and error
	Reset();

	// Show the modal
	SetVisibility(ESlateVisibility::Visible);

	// Focus password input
	if (PasswordInput)
	{
		PasswordInput->SetKeyboardFocus();
	}
}

void UPasswordInputWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
	Reset();
}

void UPasswordInputWidget::ShowError(const FString& ErrorMessage)
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(ErrorMessage));
		ErrorText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPasswordInputWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button callbacks
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &ThisClass::OnCancelButtonClicked);
	}

	if (PasswordInput)
	{
		PasswordInput->OnTextChanged.AddDynamic(this, &ThisClass::OnPasswordTextChanged);
	}

	// Initially hide the widget
	SetVisibility(ESlateVisibility::Collapsed);

	// Initialize UI state
	Reset();
}

void UPasswordInputWidget::NativeDestruct()
{
	// Unbind callbacks
	if (JoinButton)
	{
		JoinButton->OnClicked.RemoveDynamic(this, &ThisClass::OnJoinButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.RemoveDynamic(this, &ThisClass::OnCancelButtonClicked);
	}

	if (PasswordInput)
	{
		PasswordInput->OnTextChanged.RemoveDynamic(this, &ThisClass::OnPasswordTextChanged);
	}

	Super::NativeDestruct();
}

void UPasswordInputWidget::OnJoinButtonClicked()
{
	if (!PasswordInput)
	{
		return;
	}

	FString Password = PasswordInput->GetText().ToString();

	// Validate password length
	if (Password.Len() < MinPasswordLength)
	{
		ShowError(FString::Printf(TEXT("Password must be at least %d characters"), MinPasswordLength));
		return;
	}

	// Broadcast password submission
	OnPasswordSubmitted.Broadcast(TargetLobbyInfo, Password);

	// Note: Don't hide here - let parent handle success/failure
	// Parent will call Hide() on success or ShowError() on failure
}

void UPasswordInputWidget::OnCancelButtonClicked()
{
	Hide();
	OnPasswordCancelled.Broadcast();
}

void UPasswordInputWidget::OnPasswordTextChanged(const FText& Text)
{
	// Clear error when user starts typing
	if (ErrorText)
	{
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Update join button state
	UpdateJoinButtonState();
}

void UPasswordInputWidget::Reset()
{
	// Clear password input
	if (PasswordInput)
	{
		PasswordInput->SetText(FText::GetEmpty());
	}

	// Clear error text
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Update button state
	UpdateJoinButtonState();
}

void UPasswordInputWidget::UpdateJoinButtonState()
{
	if (JoinButton && PasswordInput)
	{
		FString Password = PasswordInput->GetText().ToString();
		bool bIsValid = Password.Len() >= MinPasswordLength;
		JoinButton->SetIsEnabled(bIsValid);
	}
}
