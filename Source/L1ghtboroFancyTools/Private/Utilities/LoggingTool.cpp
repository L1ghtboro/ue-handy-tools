#include "Utilities/LoggingTool.h"

DEFINE_LOG_CATEGORY(LogUniversal);

// Logging tool
void ULoggingTool::LogDebugMessage(const FString& Message, FColor Color, float Duration) {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}
	UE_LOG(LogUniversal, Log, TEXT("%s"), *Message);
}