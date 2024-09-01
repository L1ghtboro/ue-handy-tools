#include "Generators/SpawnCorridor.h"
//#include "Utilities/RoomAssetLoader.h"
//#include "Utilities/LoggingTool.h"

// Define log category for SpawnCorridor
DEFINE_LOG_CATEGORY(LogSpawnCorridor);

ASpawnCorridor::ASpawnCorridor(){
	PrimaryActorTick.bCanEverTick = true;

	// Set default parameters for corridor creation
	SetParamForwardWalls(1);
	SetParamRightWalls(1);
	SetParamStartLocation(FVector(0.f, 0.f, 0.f));
	SetParamStartRotation(FRotator(0.f, 0.f, 0.f));
	SetParamWallLength(400.f);

	CorridroTag = "Corridor";
}

// Init Assets to build room
void ASpawnCorridor::InitAssets() {
	// Lambda function to load asset classes based on the asset name
	auto LoadAssetClass = [this](const FString& AssetName) -> TSubclassOf<AActor> {
		FString Directory;

		// Find the directory of the asset by its name from the RoomAssetData array
		for (const FAssetStruct& AssetInfo : CorridorAssetData) {
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
	this->DefaultRoofClass = LoadAssetClass(TEXT("Roof"));
}

// BeginPlay is called when the game starts or when the actor is spawned
void ASpawnCorridor::BeginPlay(){
	Super::BeginPlay();
	ULoggingTool::LogDebugMessage(TEXT("Corridor BeginPlay called."));
}

// Tick is called every frame
void ASpawnCorridor::Tick(float DeltaTime){
	Super::Tick(DeltaTime);
}

// Calculate a new position based on a positioning vector, a direction vector, and lengths
FVector ASpawnCorridor::DotProduct(FVector PositioningVector, FVector SymbolVector, float WallLength = 0.f, float WallHeight = 0.f) {
	return FVector(PositioningVector.X += WallLength * SymbolVector.X, PositioningVector.Y += WallLength * SymbolVector.Y, PositioningVector.Z += WallHeight * SymbolVector.Z);
}

// Create a floor actor and add it to the corridor
void ASpawnCorridor::CreateFloor(TSubclassOf<AActor> FloorClass, FVector StartLocation, FRotator StartRotation, int32 FloorWidth, int32 FloorLength, float WallLength) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	// Calculate spawn location
	AActor* SpawnedFloor = World->SpawnActor<AActor>(FloorClass, FVector(FloorWidth * WallLength / 2 - WallLength / 2, FloorLength * WallLength / 2, -20.f) + StartLocation, StartRotation);
	if (!SpawnedFloor) {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oopsie something went wrong. Couldn't create floor!")));
		return;
	}

	SpawnedFloor->SetActorScale3D(FVector(static_cast<float>(FloorWidth), static_cast<float>(FloorLength), 1.f));
	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Corridor Floor created at location: %s"), *SpawnedFloor->GetActorLocation().ToString()));
	
	// Add to floor object list
	CorridorObjects.FloorObjectAdd(SpawnedFloor);
}

// Create a roof actor and add it to the corridor
void ASpawnCorridor::CreateRoof(TSubclassOf<AActor> RoofClass, FVector StartLocation, FRotator StartRotation, int32 RoofWidth, int32 RoofLength, float WallLength, float WallHeight, int32 WallRows) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	// Calculate spawn location
	AActor* SpawnedRoof = World->SpawnActor<AActor>(RoofClass, FVector(RoofWidth * WallLength / 2 - WallLength / 2, RoofLength * WallLength / 2, WallHeight * WallRows) + StartLocation, StartRotation);
	if (!SpawnedRoof) {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oopsie something went wrong. Couldn't create floor!")));
		return;
	}

	SpawnedRoof->SetActorScale3D(FVector(static_cast<float>(RoofWidth), static_cast<float>(RoofLength), 1.f));
	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Corridor Roof created at location: %s"), *SpawnedRoof->GetActorLocation().ToString()));

	// Add to floor object list
	CorridorObjects.RoofObjectAdd(SpawnedRoof);
}

// Create walls around the corridor
void ASpawnCorridor::CreateWall(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLighted, FVector StartLocation, FRotator StartRotation, int32 NumberOfWallsForward, int32 NumberOfWallsRight, float WallLength, float GapBetweenWalls) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	// Define vectors for wall placement
	FVector DotProductVector[] = { FVector(1.f, 1.f, 0.f), FVector(-0.5f, 0.5f, 0.f), FVector(-0.5f, -0.5f, 0.f), FVector(0.5f, -0.5f, 0.f) };
	for (int32 i = 0; i < 4; i++) {
		StartLocation = CreateWallSegment(WallClass, WallClassLighted, DotProduct(StartLocation, DotProductVector[i], i == 0 ? 0.f : WallLength), StartRotation + FRotator(0.f, 90.f * i, 0.f), i % 2 == 0 ? NumberOfWallsForward : NumberOfWallsRight, WallLength, GapBetweenWalls);
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Wall segment created at location: %s"), *StartLocation.ToString()));
	}
}

// Create segments of a wall in a direction
FVector ASpawnCorridor::CreateWallSegment(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLighted, FVector StartLocation, FRotator StartRotation, int32 NumberOfWalls, float WallLength, float GapBetweenWalls) {
	UWorld* World = GetWorld();
	if (!World) {
		return FVector::ZeroVector;
	}

	return GenerateWalls(World, WallClass, StartLocation, StartRotation, NumberOfWalls, WallLength, GapBetweenWalls, WallClassLighted);
}

FVector ASpawnCorridor::GenerateWalls(UWorld* World, TSubclassOf<AActor> WallClass, FVector StartLocation, FRotator StartRotation, int32 NumberOfWalls, float WallLength, float GapBetweenWalls, TSubclassOf<AActor> WallClassLighted) {
	if (NumberOfWalls == 1) {
		FVector Direction = abs(StartRotation.Roll) == 180 or abs(StartRotation.Roll) == 0 ? StartRotation.Vector() : FRotationMatrix(StartRotation).GetScaledAxis(EAxis::X);
		
		return StartLocation += (WallLength + GapBetweenWalls) * Direction;
	}
	
	for (int32 WallIndex = 0; WallIndex < NumberOfWalls; WallIndex++) {
		UClass* ClasstoSpawn = WallIndex % 3 == 0 and WallClassLighted != nullptr ? WallClassLighted : WallClass;

		AActor* SpawnedWall = World->SpawnActor<AActor>(ClasstoSpawn, StartLocation, StartRotation);

		CorridorObjects.FloorObjectAdd(SpawnedWall);

		// Determine direction of wall segment
		FVector Direction = abs(StartRotation.Roll) == 180 or abs(StartRotation.Roll / WallIndex) == 0 ? StartRotation.Vector() : FRotationMatrix(StartRotation).GetScaledAxis(EAxis::X);

		StartLocation += (WallLength + GapBetweenWalls) * Direction;
	}
	return StartLocation;
}

// Create the entire corridor including floor, walls, and roof
void ASpawnCorridor::CreateCorridor() {
	ULoggingTool::LogDebugMessage(TEXT("Creating corridor..."));
	AssignCorridorAssets("JSON/RoomAssets.json");
	InitAssets();
	CreateFloor(DefaultFloorClass, ParamStartLocation, ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength);
	CreateWall(DefaultWallClass, DefaultWallClassLightned, ParamStartLocation, ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength, 0.f);
	CreateRoof(DefaultRoofClass, ParamStartLocation, ParamStartRotation, ParamForwardWalls, ParamRightWalls, ParamWallLength, 430.f, 1);
	ULoggingTool::LogDebugMessage(TEXT("Corridor created successfully."), FColor::Green);
}

// Destroy all corridor objects
void ASpawnCorridor::DestroyCorridor(){
	// Creating lambda function to destory actors
	auto SafeDestroyActor = [](AActor* Actor) {
		if (Actor) {
			AsyncTask(ENamedThreads::GameThread, [Actor]() {
				if (Actor) {
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Actor Destroyed %s"), *Actor->GetName()));
					Actor->Destroy();
					UE_LOG(LogSpawnCorridor, Log, TEXT("Destroyed actor: %s"), *Actor->GetName());
				}
				});
			}
		};

	for (AActor* Wall : CorridorObjects.WallObject) {
		SafeDestroyActor(Wall);
	}
	CorridorObjects.WallObject.Empty();

	for (AActor* Roof : CorridorObjects.RoofObject) {
		SafeDestroyActor(Roof);
	}
	CorridorObjects.RoofObject.Empty();

	for (AActor* Floor : CorridorObjects.FloorObject) {
		SafeDestroyActor(Floor);
	}
	CorridorObjects.FloorObject.Empty();
}

void ASpawnCorridor::AssignCorridorAssets(const FString& FilePath) {
	// Load all room assets
	FRoomAssetStruct AllRoomAssets = URoomAssetLoader::LoadRoomAsset(FilePath);

	// Check if the specified room tag exists in the loaded assets
	if (AllRoomAssets.Rooms.Contains(this->CorridroTag)) {
		// Get the assets for the room tag
		const FRoomAssetInfo& RoomAssetInfo = AllRoomAssets.Rooms[this->CorridroTag];

		// Append the assets to RoomAssetData
		for (const FAssetStruct& AssetInfo : RoomAssetInfo.Assets) {
			CorridorAssetData.Add(AssetInfo);
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Added asset: %s"), *AssetInfo.AssetName), FColor::Green);
		}
		return;
	}

	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("No assets found for room tag: %s"), *CorridroTag), FColor::Red);
}