// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessionsTypes.h"
#include "LobbyEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryClicked, const FLobbyInfo&, LobbyInfo);

/**
 * Individual clickable lobby row displaying lobby info
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ULobbyEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Initialize with lobby data */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetLobbyInfo(const FLobbyInfo& InLobbyInfo);

	/** Get stored lobby info */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	const FLobbyInfo& GetLobbyInfo() const { return LobbyInfo; }

	/** Delegate when this entry is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnEntryClicked OnEntryClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Button handlers */
	UFUNCTION()
	void OnEntryButtonClicked();

	UFUNCTION()
	void OnEntryHovered();

	UFUNCTION()
	void OnEntryUnhovered();

private:
	/** Update display with current lobby info */
	void UpdateDisplay();

	// Bound UI Components
	UPROPERTY(meta = (BindWidget))
	UButton* EntryButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HostNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;

	UPROPERTY(meta = (BindWidget))
	UImage* LockIcon;

	UPROPERTY(meta = (BindWidget))
	UBorder* BackgroundBorder;

	// Style colors
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor NormalColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor HoveredColor = FLinearColor(0.1f, 0.1f, 0.15f, 0.8f);

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor PrivateLobbyColor = FLinearColor(0.05f, 0.02f, 0.08f, 0.5f);

	// Data
	UPROPERTY()
	FLobbyInfo LobbyInfo;
};
