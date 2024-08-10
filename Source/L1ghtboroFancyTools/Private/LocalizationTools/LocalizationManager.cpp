#include "LocalizationTools/LocalizationManager.h"
#include "Utilities/JsonLibrary.h"
#include "Utilities/LoggingTool.h"

void ULocalizationManager::Init(const FString& LanguageCode, const FString& FilePath) {
    // Load the JSON file
    if (!LoadLocalizationData(FilePath)) {
        ULoggingTool::LogDebugMessage(TEXT("Failed to load localization data."), FColor::Red);
    }
    // Set the current language
    SetCurrentLanguage(LanguageCode);
}

bool ULocalizationManager::LoadLocalizationData(const FString& FilePath) {
    // Use the JsonLibrary to load the JSON data
    LocalizationData = UJsonLibrary::LoadJSONFromFile(FilePath);

    if (!LocalizationData.IsValid()) {
        ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Failed to load JSON file: %s"), *FilePath), FColor::Red);
        return false;
    }

    return true;
}

FString ULocalizationManager::GetLocalizedString(const FString& Key) const {
    if (LocalizationData.IsValid()) {
        TSharedPtr<FJsonObject> LanguageData;
        if (UJsonLibrary::GetObjectField(LocalizationData, CurrentLanguage, LanguageData)) {
            FString LocalizedString;
            if (UJsonLibrary::GetStringField(LanguageData, Key, LocalizedString)) {
                return LocalizedString;
            }
        }
    }

    ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Key '%s' not found in language '%s'"), *Key, *CurrentLanguage), FColor::Yellow);
    return FString("Key Not Found");
}

void ULocalizationManager::SetCurrentLanguage(const FString& LanguageCode) {
    CurrentLanguage = LanguageCode;
    ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Language set to: %s"), *LanguageCode), FColor::Green);
}

TArray<FString> ULocalizationManager::GetAvailableLanguages() const {
    TArray<FString> LanguageKeys;
    if (LocalizationData.IsValid()) {
        LocalizationData->Values.GetKeys(LanguageKeys);
    }
    return LanguageKeys;
}

bool ULocalizationManager::HasKey(const FString& Key) const {
    if (LocalizationData.IsValid()) {
        TSharedPtr<FJsonObject> LanguageData;
        if (UJsonLibrary::GetObjectField(LocalizationData, CurrentLanguage, LanguageData)) {
            return LanguageData->HasField(Key);
        }
    }
    return false;
}