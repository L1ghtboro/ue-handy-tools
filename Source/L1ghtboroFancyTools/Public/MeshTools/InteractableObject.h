#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshTools/Interfaces/Interactable.h"
#include "InteractableObject.generated.h"

class UBoxComponent;

UCLASS() class L1GHTBOROFANCYTOOLS_API AInteractableObject : public AActor, public IInteractable {
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Box component for interaction detection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	UBoxComponent* InteractionBox;

	// Override the Interact function from the IInteractible interface
	virtual void Interact(AActor* InteractingActor) override;

	// Override the StartInteraction function from the IInteractible interface
	virtual void StartInteraction(AActor* InteractingActor) override;

	// Override the StopInteraction function from the IInteractible interface
	virtual void StopInteraction(AActor* InteractingActor) override;

private:
	// Handle overlap events
	UFUNCTION()
	void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
