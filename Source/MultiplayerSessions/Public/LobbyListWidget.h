// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessionsTypes.h"
#include "LobbyListWidget.generated.h"

class UMultiplayerSessionsSubsystem;
class ULobbyEntryWidget;
class UScrollBox;
class UButton;
class UTextBlock;
class UWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbySelected, const FLobbyInfo&, SelectedLobby);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackButtonPressed);

/**
 *  Widget for displaying and browsing available lobbies
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ULobbyListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Setup function called by parent Menu */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void Setup(UMultiplayerSessionsSubsystem* Subsystem);

	/** Trigger a lobby search */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void RefreshLobbyList();

	/** Delegate when user selects a lobby to join */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbySelected OnLobbySelected;

	/** Delegate when back button is pressed */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnBackButtonPressed OnBackButtonPressed;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Button Handlers */
	UFUNCTION()
	void OnRefreshButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	/** Callback from subsystem */
	UFUNCTION()
	void OnLobbyListUpdated(const TArray<FLobbyInfo>& Lobbies, bool bWasSuccessful);

	UFUNCTION()
	void OnLobbyEntryClicked(const FLobbyInfo& LobbyInfo);

private:
	/** Populate the scroll box with lobby entries */
	void PopulateLobbyList(const TArray<FLobbyInfo>& Lobbies);

	/** Clear all lobby entries */
	void ClearLobbyList();

	/** Set loading state UI */
	void SetLoadingState(bool bIsLoading);

	/** Update status text */
	void UpdateStatusText(int32 LobbyCount);

	// Bound UI Components
	UPROPERTY(meta = (BindWidget))
	UScrollBox* LobbyScrollBox;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UWidget* LoadingIndicator;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EmptyStateText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	// Configuration
	// NOT IMPLEMENTED YET
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TSubclassOf<ULobbyEntryWidget> LobbyEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	int32 MaxSearchResults = 100;

	// State
	UPROPERTY()
	UMultiplayerSessionsSubsystem* SessionsSubsystem;

	// NOT IMPLEMENTED YET
	UPROPERTY()
	TArray<ULobbyEntryWidget*> LobbyEntryWidgets;
};
