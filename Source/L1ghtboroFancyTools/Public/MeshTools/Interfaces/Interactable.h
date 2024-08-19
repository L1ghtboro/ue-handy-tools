#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(MinimalAPI) class UInteractable : public UInterface {
	GENERATED_BODY()
};

class IInteractable {
	GENERATED_BODY()

public:
	// Define the functions that all interactible objects must implement
	virtual void Interact(AActor* InteractingActor) = 0;
	virtual void StartInteraction(AActor* InteractingActor) = 0;
	virtual void StopInteraction(AActor* InteractingActor) = 0;
};