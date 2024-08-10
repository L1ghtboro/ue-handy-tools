#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LoggingTool.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniversal, Log, All);

UCLASS() class L1GHTBOROFANCYTOOLS_API ULoggingTool : public UObject {
	GENERATED_BODY()
	
public:
	static void LogDebugMessage(const FString& Message, FColor Color = FColor::White, float Duration = 5.f);
};
