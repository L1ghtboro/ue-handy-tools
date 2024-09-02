#include "Generators/SpawnRoom.h"
#include "Async/Async.h"
#include "Utilities/LoggingTool.h"

DEFINE_LOG_CATEGORY(LogSpawnRoom);

int32 ASpawnRoom::RoomCount = 0;

typedef void (ASpawnRoom::* RoomCreationMethod)(FVector, float, int32, int32);

ASpawnRoom::ASpawnRoom(){
	PrimaryActorTick.bCanEverTick = true;
	bIsPlayerInRoom = false;
	
	RoomID = -1;

	PlayerActor = nullptr;

	SetParamForwardWalls(static_cast<int32>(FMath::FRandRange(3.f, 12.f)));
	SetParamRightWalls(static_cast<int32>(FMath::FRandRange(3.f, 12.f)));
	SetParamWallLength(400.f);
	SetParamStartLocation(FVector(0.f, 0.f, 0.f));
	SetParamStartRotation(FRotator(0.f, 0.f, 0.f));

	// Create and set the root component
	RootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));

	// Create the player detector box
	PlayerDetectBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDetectBox"));
	PlayerDetectBox->InitBoxExtent(FVector(200.f, 200.f, 200.f));
	PlayerDetectBox->SetCollisionProfileName(TEXT("Trigger"));
	PlayerDetectBox->OnComponentBeginOverlap.AddDynamic(this, &ASpawnRoom::OnOverlapBegin);
	PlayerDetectBox->OnComponentEndOverlap.AddDynamic(this, &ASpawnRoom::OnOverlapEnd);
}

ASpawnRoom::~ASpawnRoom(){
	//RoomAssetData.Empty();
}

// Initialize room parameters
void ASpawnRoom::Init(int32 ParamForwardWallsNew, int32 ParamRightWallsNew, float ParamWallLengthNew, FVector ParamStartLocationNew, FRotator ParamStartRotationNew) {
	this->ParamForwardWalls  = ParamForwardWallsNew;
	this->ParamRightWalls    = ParamRightWallsNew;
	this->ParamWallLength    = ParamWallLengthNew;
	this->ParamStartLocation = ParamStartLocationNew;
	this->ParamStartRotation = ParamStartRotationNew;
}

// Initialize room parameters by copying from another class
void ASpawnRoom::InitByClass(ASpawnRoom* Class){
	this->ParamForwardWalls  = Class->GetParamForwardWalls();
	this->ParamRightWalls    = Class->GetParamRightWalls();
	this->ParamWallLength    = Class->GetParamWallLength();
	this->ParamStartLocation = Class->GetParamStartLocation();
	this->ParamStartRotation = Class->GetParamStartRotation();
}

// Called when the game starts or when spawned
void ASpawnRoom::BeginPlay(){
	Super::BeginPlay();
	// Attach the player detector box to the root component
	PlayerDetectBox->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	UE_LOG(LogSpawnRoom, Log, TEXT("Room BeginPlay called."));
	
	this->SetParamRoomID(RoomCount++);

	EntranceProbability = 0.3f;
}

// Called every frame
void ASpawnRoom::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

// Calculate the dot product of two vectors with optional wall length and height adjustments
FVector ASpawnRoom::DotProduct(FVector PositioningVector, FVector SymbolVector, float WallLength = 0.f, float WallHeight = 0.f) {
	return FVector(PositioningVector.X += WallLength * SymbolVector.X, PositioningVector.Y += WallLength * SymbolVector.Y, PositioningVector.Z += WallHeight * SymbolVector.Z);
}

// Determine if a wall will be an entrance based on a random value
bool ASpawnRoom::bWillBeEntrance() {
	return FMath::FRandRange(0.f, 1.0f) <= EntranceProbability;
}

// Generate an entrance structure for a wall
FEntranceStruct ASpawnRoom::GenerateEntranceStruct(float NumberOfWalls, TSubclassOf<AActor> EntranceClass, FEntranceStruct EntranceStruct) {
	if (!EntranceStruct.bWillBeEntrance) {
		bool bWillBeEntranceValue = bWillBeEntrance();
		FEntranceStruct Entrance(EntranceClass, bWillBeEntranceValue ? FMath::FRandRange(1, NumberOfWalls - 1) : -1, bWillBeEntranceValue, FVector::ZeroVector, FRotator::ZeroRotator);
		return Entrance;
	}
	return EntranceStruct;
}

// Generate an entrance structure at a specific position
FEntranceStruct ASpawnRoom::GenerateEntranceStruct(FVector Position, TSubclassOf<AActor> EntranceClass, FEntranceStruct EntranceStruct){
	if (!EntranceStruct.bWillBeEntrance) {
		bool bWillBeEntranceValue = bWillBeEntrance();
		FEntranceStruct Entrance(EntranceClass, 0, bWillBeEntranceValue, Position, FRotator::ZeroRotator);
		return Entrance;
	}
	return EntranceStruct;
}

// Create the floor of the room
void ASpawnRoom::CreateFloor(TSubclassOf<AActor> FloorClass, FVector StartLocation, FRotator StartRotation, int32 FloorWidth, int32 FloorLength, float WallLength) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	AActor* SpawnedFloor = World->SpawnActor<AActor>(FloorClass, FVector(FloorWidth * WallLength / 2 - WallLength / 2, FloorLength * WallLength / 2, -20.f) + StartLocation, StartRotation);
	if (!SpawnedFloor) {
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]() {
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oopsie something went wrong. Couldn't create floor!")));
			});
		return;
	}

	SpawnedFloor->SetActorScale3D(FVector(static_cast<float>(FloorWidth), static_cast<float>(FloorLength), 1.f)); //Adjusting Floor scale

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, SpawnedFloor]() {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Room Floor created at location: %s"), *SpawnedFloor->GetActorLocation().ToString()));
		RoomObjects.FloorObjectAdd(SpawnedFloor);
		});
}

// Create the roof of the room
void ASpawnRoom::CreateRoof(TSubclassOf<AActor> RoofClass, FVector StartLocation, FRotator StartRotation, int32 RoofWidth, int32 RoofLength, float WallLength) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	AActor* SpawnedRoof = World->SpawnActor<AActor>(RoofClass, FVector(RoofWidth * WallLength / 2 - WallLength / 2, RoofLength * WallLength / 2, 880.f) + StartLocation, StartRotation);
	if (!SpawnedRoof) {
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]() {
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oopsie smothing went wrong. Couldn't create roof!")));
			});
		return;
	}

	SpawnedRoof->SetActorScale3D(FVector(static_cast<float>(RoofWidth), static_cast<float>(RoofLength), 1.f)); //Adjusting Roof scale

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, SpawnedRoof]() {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Room Roof created at location: %s"), *SpawnedRoof->GetActorLocation().ToString()));
		RoomObjects.RoofObjectAdd(SpawnedRoof);
		});
}

FVector ASpawnRoom::GenerateWalls(UWorld* World, TSubclassOf<AActor> WallClass, FVector StartLocation, FRotator StartRotation, int32 NumberOfWalls, float WallLength, float GapBetweenWalls, FEntranceStruct EntranceObject, TSubclassOf<AActor> WallClassLightned) {
	for (int32 WallIndex = 0; WallIndex < NumberOfWalls; WallIndex++) {
		// Determine if we should spawn an entrance
		bool bIsEntrance = EntranceObject.bWillBeEntrance and (EntranceObject.EntranceID == WallIndex or (EntranceObject.EntrancePosition == StartLocation and EntranceObject.EntrancePosition != FVector::ZeroVector)) and EntranceObject.EntranceClass != nullptr;

		// Determine if we should spawn a lightened wall
		bool bIsLightenedWall = WallIndex % 3 == 0 and WallClassLightned != nullptr;

		// Select the class to spawn based on conditions
		UClass* ClassToSpawn = bIsEntrance ? EntranceObject.EntranceClass : bIsLightenedWall ? WallClassLightned : WallClass;

		// Spawn the selected actor class
		AActor* SpawnedWall = World->SpawnActor<AActor>(ClassToSpawn, StartLocation, StartRotation);

		// Add to the appropriate list
		ClassToSpawn == EntranceObject.EntranceClass ? RoomObjects.EntranceObjectAdd(SpawnedWall) : RoomObjects.WallObjectAdd(SpawnedWall);

		// Determine the direction for the next wall placement
		FVector Direction = (abs(StartRotation.Roll) == 180 && abs(StartRotation.Roll / WallIndex) == 0) ? StartRotation.Vector() : FRotationMatrix(StartRotation).GetScaledAxis(EAxis::X);

		// Update the start location for the next wall
		StartLocation += (WallLength + GapBetweenWalls) * Direction;
	}

	return StartLocation;
}


// Create a segment of the wall with optional entrances
FVector ASpawnRoom::CreateWallSegment(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, FEntranceStruct& EntranceObject, FVector StartLocation, FRotator StartRotation, int32 NumberOfWalls, float WallLength, float GapBetweenWalls) {
	UWorld* World = GetWorld();
	if (!World) {
		return FVector::ZeroVector;
	}

	return GenerateWalls(World, WallClass, StartLocation, StartRotation, NumberOfWalls, WallLength, GapBetweenWalls, EntranceObject, WallClassLightned);
}

// Create the walls of the room
void ASpawnRoom::CreateWall(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, TSubclassOf<AActor> EntranceClass, FVector StartLocation, FRotator StartRotation, int32 NumberOfWallsForward, int32 NumberOfWallsRight, float WallLength, float GapBetweenWalls, FEntranceStruct EntranceObject) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	FVector DotProductVector[] = { FVector(1.f, 1.f, 0.f), FVector(-0.5f, 0.5f, 0.f), FVector(-0.5f, -0.5f, 0.f), FVector(0.5f, -0.5f, 0.f) };

	for (int32 WallSide = 0; WallSide < 4; WallSide++) {
		bool IsEntranceInThisPlane = false;

		// FMath::IsNearlyEqual needs to check if the angle is -1 or +1 diff
		if (FMath::IsNearlyEqual(EntranceObject.EntranceRotation.Yaw, WallSide * 90.f, 1.f) or (FMath::IsNearlyEqual(EntranceObject.EntranceRotation.Yaw, -90.f, 1.f) and WallSide == 3) and EntranceObject.EntranceRotation != FRotator::ZeroRotator) {
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("New entrance generated for plane %d %s"), EntranceObject.EntranceID, *EntranceObject.EntrancePosition.ToString()));

			EntranceObject.EntranceClass = DefaultEntranceClass;

			StartLocation = CreateWallSegment(WallClass, WallClassLightned, EntranceObject, DotProduct(StartLocation, DotProductVector[WallSide], WallSide == 0 ? 0.f : WallLength), StartRotation + FRotator(0.f, 90.f * WallSide, 0.f), WallSide % 2 == 0 ? NumberOfWallsForward : NumberOfWallsRight, WallLength, GapBetweenWalls);
		}
		else {
			FEntranceStruct EntranceStruct = GenerateEntranceStruct(WallSide % 2 == 0 ? NumberOfWallsForward : NumberOfWallsRight, EntranceClass, FEntranceStruct());

			StartLocation = CreateWallSegment(WallClass, WallClassLightned, EntranceStruct, DotProduct(StartLocation, DotProductVector[WallSide], WallSide == 0 ? 0.f : WallLength), StartRotation + FRotator(0.f, 90.f * WallSide, 0.f), WallSide % 2 == 0 ? NumberOfWallsForward : NumberOfWallsRight, WallLength, GapBetweenWalls);
		}

		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Wall segment created at location: %s"), *StartLocation.ToString()));
	}
}

// Init Assets to build room
void ASpawnRoom::InitAssets() {
	// Lambda function to load asset classes based on the asset name
	auto LoadAssetClass = [this](const FString& AssetName) -> TSubclassOf<AActor> {
		FString Directory;

		// Find the directory of the asset by its name from the RoomAssetData array
		for (const FAssetStruct& AssetInfo : RoomAssetData) {
			if (AssetInfo.AssetName == AssetName) {
				Directory = AssetInfo.Directory;
				break;
			}
		}

		// Check if the directory is found, log an error message if not
		if (Directory.IsEmpty()) {
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Asset not found: %s"), *AssetName), FColor::Red);
			return nullptr;
		}

		// Load the class object from the asset directory
		UClass* BlueprintObject = StaticLoadClass(UObject::StaticClass(), nullptr, *Directory);
		if (!BlueprintObject) {
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Failed to load asset: %s"), *Directory), FColor::Red);
			return nullptr;
		}

		// Cast the loaded object to UBlueprintGeneratedClass
		UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(BlueprintObject);
		if (!BlueprintClass) {
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Invalid blueprint class: %s"), *Directory), FColor::Red);
			return nullptr;
		}

		// Return the casted blueprint class
		return BlueprintClass;
		};

	// Load asset classes for the default components of the room
	this->DefaultFloorClass = LoadAssetClass(TEXT("Floor"));
	this->DefaultWallClass = LoadAssetClass(TEXT("Wall"));
	this->DefaultWallClassLightned = LoadAssetClass(TEXT("WallLighted"));
	this->DefaultEntranceClass = LoadAssetClass(TEXT("Entrance"));
	this->DefaultRoofClass = LoadAssetClass(TEXT("Roof"));
}

// Create whole room method
void ASpawnRoom::CreateRoom(FEntranceStruct EntranceInfo){
	ULoggingTool::LogDebugMessage(TEXT("Creating room..."));
	// Assign room tag
	if (RoomTag.IsEmpty()) {
		AssignRoomTag("JSON/RoomTags.json");
	}
	AssignRoomAssets("JSON/RoomAssets.json");
	InitAssets();
	CreateFloor(DefaultFloorClass, ParamStartLocation, ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength);
	CreateWall(DefaultWallClass, DefaultWallClassLightned, DefaultEntranceClass, ParamStartLocation, ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength, 0.f, EntranceInfo);
	CreateWall(DefaultWallClass, nullptr, nullptr, ParamStartLocation + FVector(0.f, 0.f, 450.f), ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength, 0.f); //Create upper walls without Lighting assets
	CreateRoof(DefaultRoofClass, ParamStartLocation, ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength);
	ULoggingTool::LogDebugMessage(TEXT("Room created successfully."), FColor::Green);
	ULoggingTool::LogDebugMessage(TEXT("Attaching detection component..."));
	CreatePlayerDetector();
	ULoggingTool::LogDebugMessage(TEXT("Detection component attached."), FColor::Green);

}

// Destroy room and emptying room elements
void ASpawnRoom::DestroyRoom() {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	// Creating lambda function to destory actors
	auto SafeDestroyActor = [](AActor* Actor) {
		if (Actor) {
			AsyncTask(ENamedThreads::GameThread, [Actor]() {
				if (Actor) {
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Actor Destroyed %s"), *Actor->GetName()));
					Actor->Destroy();
					UE_LOG(LogSpawnRoom, Log, TEXT("Destroyed actor: %s"), *Actor->GetName());
				}
				});
		}
		};

	for (AActor* Wall : RoomObjects.WallObject) {
		SafeDestroyActor(Wall);
	}
	RoomObjects.WallObject.Empty(); // Emptying array

	for (AActor* Entrance : RoomObjects.EntranceObject) {
		SafeDestroyActor(Entrance);
	}
	RoomObjects.EntranceObject.Empty();

	for (AActor* Floor : RoomObjects.FloorObject) {
		SafeDestroyActor(Floor);
	}
	RoomObjects.FloorObject.Empty();

	for (AActor* Roof : RoomObjects.RoofObject) {
		SafeDestroyActor(Roof);
	}
	RoomObjects.RoofObject.Empty();

	for (AActor* Prop : RoomObjects.PropObject) {
		SafeDestroyActor(Prop);
	}
	RoomObjects.PropObject.Empty();
}

// Add to the room BoxComponent to detect if player in room
void ASpawnRoom::CreatePlayerDetector(){
	FVector DetectorLocation = ParamStartLocation + FVector(ParamForwardWalls * ParamWallLength / 2, ParamRightWalls * ParamWallLength / 2, 50.f);
	FRotator DetectRotation = ParamStartRotation;
	PlayerDetectBox->SetWorldLocation(DetectorLocation);
	PlayerDetectBox->SetWorldRotation(DetectRotation);
	PlayerDetectBox->SetBoxExtent(FVector(ParamForwardWalls * ParamWallLength / 2, ParamRightWalls * ParamWallLength / 2, 50.f)); // Setting Box as long as our room
}

//Assign tag to room
void ASpawnRoom::AssignRoomTag(const FString& FilePath) {
	// JSON load object
	TArray<FRoomCategoryStruct> RoomCategories = URoomTagLoader::LoadRoomTags(FilePath);

	// Create Map to store category probabilities
	TMap<FString, float> CategoryWeights;
	float TotalWeights = 0.f;
	for (const FRoomCategoryStruct& Category : RoomCategories) {
		float CumulativeWeight = 0.f;
		for (const FRoomTagStruct& Tag : Category.Tags) {
			CumulativeWeight += Tag.Weight;
		}
		TotalWeights += CumulativeWeight;
		CategoryWeights.Add(Category.CategoryName, CumulativeWeight);
	}

	// Choose category by the probability
	float RandomValue = FMath::FRandRange(0.f, TotalWeights);
	FString ChosenCategory;
	for (const auto& Elem : CategoryWeights) {
		RandomValue -= Elem.Value;
		if (RandomValue <= Elem.Value) {
			ChosenCategory = Elem.Key;
			break;
		}
	}

	// Find the chosen category in RoomCategories
	FRoomCategoryStruct* SelectedCategory = RoomCategories.FindByPredicate([&ChosenCategory](const FRoomCategoryStruct& Category) {
		return Category.CategoryName == ChosenCategory;
		});

	if (!SelectedCategory) {
		UE_LOG(LogTemp, Error, TEXT("Failed to find selected category: %s"), *ChosenCategory);
		return;
	}

	// Create map to store tag probabilities within the chosen category
	TMap<FString, float> TagWeights;
	float TotalTagWeights = 0.f;
	for (const FRoomTagStruct& Tag : SelectedCategory->Tags) {
		TagWeights.Add(Tag.Name, Tag.Weight);
		TotalTagWeights += Tag.Weight;
	}

	// Choose tag by the probability within the chosen category
	RandomValue = FMath::FRandRange(0.f, TotalTagWeights);
	for (const auto& Elem : TagWeights) {
		RandomValue -= Elem.Value;
		if (RandomValue <= 0.f) {
			RoomTag = Elem.Key;
			break;
		}
	}

	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Room Tag for current - ID %d: %s"), RoomID, *RoomTag), FColor::Yellow);
}

void ASpawnRoom::AssignRoomAssets(const FString& FilePath){
	// Load all room assets
	FRoomAssetStruct AllRoomAssets = URoomAssetLoader::LoadRoomAsset(FilePath);
	
	// Check if the specified room tag exists in the loaded assets
	if (AllRoomAssets.Rooms.Contains(this->RoomTag)) {
		// Get the assets for the room tag
		const FRoomAssetInfo& RoomAssetInfo = AllRoomAssets.Rooms[this->RoomTag];

		// Append the assets to RoomAssetData
		for (const FAssetStruct& AssetInfo : RoomAssetInfo.Assets) {
			RoomAssetData.Add(AssetInfo);
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Added asset: %s"), *AssetInfo.AssetName), FColor::Green);
		}
		return;
	}

	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("No assets found for room tag: %s"), *RoomTag), FColor::Red);
}

// Event handler for when a player enters the detection box
void ASpawnRoom::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult){
	if (!(OtherActor && (OtherActor != this) && OtherComp and !bIsPlayerInRoom)) {
		return;
	}
	
	bIsPlayerInRoom = true;
	PlayerActor = OtherActor;
	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Player entered room: %s, Current Room ID is %d"), *OtherActor->GetName(), RoomID), FColor::Green);
	GetWorld()->GetTimerManager().SetTimer(PlayerCheckTimerHandle, this, &ASpawnRoom::CheckPlayerPosition, 0.01f, true); //Adding timed method to check if player at the high ground or just jumping
}

// Event handler for when a player exits the detection box
void ASpawnRoom::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){
	if (OtherActor && (OtherActor != this) && OtherComp) {
		GetWorld()->GetTimerManager().SetTimer(PlayerCheckTimerHandle, this, &ASpawnRoom::CheckPlayerPosition, 0.01f, false);
	}
}

// Method to check the player's position relative to the room
void ASpawnRoom::CheckPlayerPosition(){
	// Check if the current thread is the game thread
	if (!IsInGameThread()) {
		// If not, asynchronously execute this function on the game thread
		AsyncTask(ENamedThreads::GameThread, [this](){
				CheckPlayerPosition();
			});
		return;
	}
	// If PlayerActor is valid, proceed to check the player's position
	if (PlayerActor) {
		FVector PlayerLocation = PlayerActor->GetActorLocation();  // Get the player's current location
		FVector BoxLocation = PlayerDetectBox->GetComponentLocation();  // Get the detection box's location
		FVector BoxExtent = PlayerDetectBox->GetScaledBoxExtent();  // Get the detection box's extent

		// Check if the player is outside the detection box and not within the extended vertical margin
		if (!PlayerDetectBox->IsOverlappingActor(PlayerActor) && !IsPointInsideBox(PlayerLocation, BoxLocation, BoxExtent, 400.f)) {
			bIsPlayerInRoom = false;  // Set flag indicating the player is not in the room
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Player exited room: %s, Current Room ID is %d"), *PlayerActor->GetName(), RoomID), FColor::Red);  // Log player exit
			GetWorld()->GetTimerManager().ClearTimer(PlayerCheckTimerHandle);  // Clear the player check timer
			PlayerActor = nullptr;  // Reset PlayerActor to nullptr
		}
	}
}

// Method to check if a point is inside a given box with an additional vertical margin
bool ASpawnRoom::IsPointInsideBox(const FVector& Point, const FVector& BoxCenter, const FVector& BoxExtent, float VerticalMargin){ //Checking if player is in box + adding to it VerticalMargin
	return FMath::Abs(Point.X - BoxCenter.X) <= BoxCenter.X && FMath::Abs(Point.Y - BoxCenter.Y) <= BoxCenter.Y && FMath::Abs(Point.Z - BoxCenter.Z) <= (BoxCenter.Z + VerticalMargin);
}

UClass* ASpawnRoom::LoadAssetClass(const FString& AssetPath) {
	if (AssetPath.IsEmpty()) {
		return nullptr;
	}
	UClass* AssetClass = StaticLoadClass(UObject::StaticClass(), nullptr, *AssetPath);
	if (!AssetClass) {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Failed to load asset: %s"), *AssetPath), FColor::Red);
	}	
	return AssetClass;
}

void ASpawnRoom::CreateSingleProp(UClass* PropClass, FVector StartLocation, FRotator StartRotation) {
	UWorld* World = GetWorld();
	if (!World || !PropClass) {
		return;
	}

	// Spawn single prop asynchronously
	AsyncTask(ENamedThreads::GameThread, [this, PropClass, StartLocation, StartRotation, World]() {
		AActor* SpawnedActor = World->SpawnActor<AActor>(PropClass, StartLocation, StartRotation);
		if (!SpawnedActor) {
			ULoggingTool::LogDebugMessage(TEXT("Failed to spawn prop"), FColor::Red);
		}
		else {
			RoomObjects.PropObjectAdd(SpawnedActor);
		}
		});
}
