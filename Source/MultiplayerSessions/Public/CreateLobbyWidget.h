// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessionsTypes.h"
#include "CreateLobbyWidget.generated.h"

class UMultiplayerSessionsSubsystem;
class UEditableTextBox;
class USpinBox;
class UCheckBox;
class UButton;
class UTextBlock;
class UWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCreationComplete, bool, bSuccess);

/**
 * Widget for configuring and creating a new lobby
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UCreateLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Setup function called by parent Menu */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void Setup(UMultiplayerSessionsSubsystem* Subsystem);

	/** Delegate for notifying parent when lobby is created or canceled */
	FOnLobbyCreationComplete OnLobbyCreationComplete;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Button Handlers */
	UFUNCTION()
	void OnCreateButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	UFUNCTION()
	void OnPublicCheckBoxChanged(bool bIsChecked);

	/** Callback from subsystem */
	UFUNCTION()
	void OnLobbyCreated(bool bWasSuccessful, const FLobbyInfo& LobbyInfo);

private:
	/** Validates input and returns true if valid */
	bool ValidateInput();

	/** Updates UI based on public/private state */
	void UpdatePasswordVisibility();

	/** Sets the loading state of the widget */
	void SetLoadingState(bool bIsLoading);

	/** Shows an error message */
	void ShowError(const FString& Message);

	/** Clears the error message */
	void ClearError();

	// Bound UI Components
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* LobbyNameInput;

	UPROPERTY(meta = (BindWidget))
	USpinBox* MaxPlayersSpinBox;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* PublicCheckBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordInput;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ErrorText;

	UPROPERTY(meta = (BindWidget))
	UWidget* PasswordContainer;

	// State
	UPROPERTY()
	UMultiplayerSessionsSubsystem* SessionsSubsystem;
	bool bIsCreatingLobby = false;
};
