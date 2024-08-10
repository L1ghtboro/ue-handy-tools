#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LocalizationManager.generated.h"

UCLASS() class L1GHTBOROFANCYTOOLS_API ULocalizationManager : public UObject {
	GENERATED_BODY()
	
private:
	TSharedPtr<FJsonObject> LocalizationData;
	FString CurrentLanguage;

public:
	// Initializae the Localization Manager
	void Init(const FString& LanguageCode, const FString& FilePath);

	// Load localization data from JSON
	bool LoadLocalizationData(const FString& FilePath);

	// Get Localization string by key
	FString GetLocalizedString(const FString& Key) const;

	// Set the current language
	void SetCurrentLanguage(const FString& LanguageCode);

	// Get the current language
	FString GetCurrentLanguage() const { return CurrentLanguage; }

	// Get all available languages
	TArray<FString> GetAvailableLanguages() const;

	// Check if a key exists
	bool HasKey(const FString& Key) const;
};
