// LobbyHUDWidget.h
// Main UI widget displayed when players are in the lobby

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyHUDWidget.generated.h"

class ALobbyGameState;
class ALobbyPlayerState;
class ULobbyPlayerEntryWidget;
class UButton;
class UTextBlock;
class UScrollBox;

/**
 * Main HUD widget for the lobby screen.
 * Displays player list, ready status controls, and start game button for host.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ULobbyHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initialize the widget. Call this after adding to viewport.
	 * Can be called from Blueprint after spawning the widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InitializeLobbyHUD();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void MenuSetup();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void MenuTearDown();

	/** Manually refresh the player list */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void RefreshPlayerList();

	/** Check if this client is the host */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsLocalPlayerHost() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Button Handlers
	UFUNCTION()
	void OnReadyButtonClicked();

	UFUNCTION()
	void OnStartGameButtonClicked();

	UFUNCTION()
	void OnLeaveButtonClicked();

	// GameState event handlers
	UFUNCTION()
	void OnPlayerReadyStateChanged(ALobbyPlayerState* PlayerState, bool bIsReady);

	UFUNCTION()
	void OnAllPlayersReady();

	UFUNCTION()
	void OnNotAllPlayersReady();

private:
	void BindToGameState();
	void UnbindFromGameState();
	void RebuildPlayerList();
	void UpdateReadyButton();
	void UpdateStartButton();
	void UpdateStatusText();
	ALobbyPlayerState* GetLocalPlayerState() const;

	class ALobbyGameMode* LobbyGameMode;

	// Bound UI Components

	/** Container for player entry widgets */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerListScrollBox;

	/** Ready/Unready toggle button */
	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;

	/** Text on ready button ("Ready" / "Cancel") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyButtonText;

	/** Start Game button - only visible/enabled for host */
	UPROPERTY(meta = (BindWidget))
	UButton* StartGameButton;

	/** Text on start button */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StartGameButtonText;

	/** Leave Lobby button */
	UPROPERTY(meta = (BindWidget))
	UButton* LeaveButton;

	/** Status text showing "Waiting for players..." or "All Ready!" */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	/** Player count display "3/4 Players" */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* PlayerCountText;

	/** Title/Header text */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* LobbyTitleText;

	// Configuration

	/** Widget class to spawn for each player entry */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TSubclassOf<ULobbyPlayerEntryWidget> PlayerEntryWidgetClass;

	/** Path to the game level to travel to when starting */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	FString GameLevelPath = TEXT("/Game/Maps/Lvl_Old_Lobby?listen");

	// State

	UPROPERTY()
	TArray<ULobbyPlayerEntryWidget*> PlayerEntryWidgets;

	UPROPERTY()
	ALobbyGameState* CachedGameState = nullptr;

	/** Timer for periodic refresh (handles late joiners, etc.) */
	float RefreshTimer = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	float RefreshInterval = 1.0f;

	bool bIsInitialized = false;
};
