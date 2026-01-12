// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessionsTypes.h"
#include "PasswordInputWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPasswordSubmitted, const FLobbyInfo&, LobbyInfo, const FString&,
                                             Password);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPasswordCancelled);

/**
 * Modal dialog for entering password when joining private lobbies
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UPasswordInputWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Show the dialog for a specific lobby */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowForLobby(const FLobbyInfo& InLobbyInfo);

	/** Hide and reset */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void Hide();

	/** Show error message (e.g., wrong password) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowError(const FString& ErrorMessage);

	/** Delegates */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPasswordSubmitted OnPasswordSubmitted;

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPasswordCancelled OnPasswordCancelled;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Button Handlers */
	UFUNCTION()
	void OnJoinButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	UFUNCTION()
	void OnPasswordTextChanged(const FText& Text);

private:
	/** Clear password and error */
	void Reset();

	/** Update join button state */
	void UpdateJoinButtonState();

	// Bound UI Components
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LobbyNameText;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordInput;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ErrorText;

	UPROPERTY(meta = (BindWidget))
	UBorder* ModalBackground;

	// State
	UPROPERTY()
	FLobbyInfo TargetLobbyInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Validation")
	int32 MinPasswordLength = 4;
};
