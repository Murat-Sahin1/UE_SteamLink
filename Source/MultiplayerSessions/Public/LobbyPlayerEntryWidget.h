// LobbyPlayerEntryWidget.h
// Individual player row in the lobby player list

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerEntryWidget.generated.h"

class ALobbyPlayerState;
class UTextBlock;
class UImage;
class UBorder;

/**
 * Widget representing a single player entry in the lobby player list.
 * Displays player name, ready status, and host indicator.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ULobbyPlayerEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initialize the widget with player state reference.
	 * Call this after creating the widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetPlayerState(ALobbyPlayerState* InPlayerState);

	/** Get the associated player state */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	ALobbyPlayerState* GetPlayerState() const { return PlayerState; }

	/** Manually refresh the display (called automatically on ready state changes) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void RefreshDisplay();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Called when the player's ready state changes */
	UFUNCTION()
	void OnPlayerReadyStateChanged(ALobbyPlayerState* ChangedPlayerState, bool bIsReady);

private:
	void UpdateDisplay();

	// Bound UI Components
	/** Player's display name */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	/** Ready status text (e.g., "Ready" / "Not Ready") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyStatusText;

	/** Host crown/indicator icon - visible only for host */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* HostIcon;

	/** Ready checkmark icon - visible when ready */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ReadyIcon;

	/** Background border for styling */
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* BackgroundBorder;

	// Style colors
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor ReadyColor = FLinearColor(0.0f, 0.5f, 0.0f, 0.3f);

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor NotReadyColor = FLinearColor(0.5f, 0.5f, 0.0f, 0.3f);

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor LocalPlayerColor = FLinearColor(0.0f, 0.3f, 0.5f, 0.3f);

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor ReadyTextColor = FSlateColor(FLinearColor::Green);

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor NotReadyTextColor = FSlateColor(FLinearColor::Yellow);

	// Cached player state
	UPROPERTY()
	ALobbyPlayerState* PlayerState = nullptr;

	// Whether this entry represents the local player
	bool bIsLocalPlayer = false;
};
