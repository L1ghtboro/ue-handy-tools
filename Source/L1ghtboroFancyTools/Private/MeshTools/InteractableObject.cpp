#include "MeshTools/InteractableObject.h"
#include "Components/BoxComponent.h"
// #include "Characters/MainCharacter.h" Character example
#include "Utilities/LoggingTool.h"

// Sets default values
AInteractableObject::AInteractableObject() : InteractionBox(CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"))) {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create and configure the interaction bo
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->InitBoxExtent(FVector(200.f, 200.f, 200.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void AInteractableObject::BeginPlay() {
	Super::BeginPlay();

	// Bind overlap events
	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractableObject::OnInteractionBoxBeginOverlap);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AInteractableObject::OnInteractionBoxEndOverlap);
}

// Called every frame
void AInteractableObject::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AInteractableObject::Interact(AActor* InteractingActor) {
	// To be implemented by child classes
}

void AInteractableObject::StartInteraction(AActor* InteractingActor) {
	// To be implemented by child classes
}

void AInteractableObject::StopInteraction(AActor* InteractingActor) {
	// To be implemented by child classes
}

// Here implement your character must contain SetCurrentInteractableObj methode
void AInteractableObject::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	//ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	//if (PlayerCharacter) {
	//	PlayerCharacter->SetCurrentInteractableObj(this);
	//	StartInteraction(PlayerCharacter);
	//}
}

// Here implement your character must contain SetCurrentInteractableObj methode
void AInteractableObject::OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	//ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	//if (PlayerCharacter) {
	//	PlayerCharacter->SetCurrentInteractableObj(nullptr);
	//	StopInteraction(PlayerCharacter);
	//}
}