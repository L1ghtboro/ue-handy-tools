#include "Generators/SpawnDungeon.h"
#include "Utilities/LoggingTool.h"

DEFINE_LOG_CATEGORY(LogSpawnDungeon);

ASpawnDungeon::ASpawnDungeon(){
	PrimaryActorTick.bCanEverTick = true;

	CurrentRoomID = 0;
	RoomSpawned = 0;
}

void ASpawnDungeon::BeginPlay(){
	Super::BeginPlay();

	// Start the timer to continuously check the player's room
	GetWorld()->GetTimerManager().SetTimer(DungeonCheckTimerHandle, this, &ASpawnDungeon::GenerateDungeonEternal, 1.f, true);
	ULoggingTool::LogDebugMessage(TEXT("BeginPlay: Dungeon generation started."));

	GenerateDungeonOnBoot();
}

void ASpawnDungeon::Tick(float DeltaTime){
	Super::Tick(DeltaTime);
}

void ASpawnDungeon::InitAssets(TSubclassOf<AActor> FloorClass, TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, TSubclassOf<AActor> EntranceClass, TSubclassOf<AActor> RoofClass){
	// Store the class types for later use
	this->DefaultFloorClass = FloorClass;
	this->DefaultWallClass = WallClass;
	this->DefaultWallClassLightned = WallClassLightned;
	this->DefaultEntranceClass = EntranceClass;
	this->DefaultRoofClass = RoofClass;
}

// Generates the initial dungeon layout at game start
void ASpawnDungeon::GenerateDungeonOnBoot() {
	UWorld* World = GetWorld();
	if (World) {
		FVector StartLocation = FVector::ZeroVector;
		FRotator StartRotation = FRotator::ZeroRotator;

		if (RoomDungeon.IsEmpty()) {
			// Spawn the initial room
			ASpawnRoom* RoomInitial = World->SpawnActor<ASpawnRoom>(ASpawnRoom::StaticClass(), StartLocation, StartRotation);

			RoomInitial->SetParamRoomTag("Initial Room");

			if (RoomInitial) {
				// Create an entrance structure for the initial room
				FEntranceStruct InitialEntrance = FEntranceStruct(DefaultEntranceClass, 0, true, FVector::ZeroVector, FRotator::ZeroRotator);

				RoomInitial->CreateRoom(InitialEntrance);
				ULoggingTool::LogDebugMessage(TEXT("Initial Room Created"));
			}

			// Assign Room ID and set it as the current room
			CurrentRoomID = RoomInitial->GetParamRoomID();

			RoomDungeon.Add(RoomInitial);

			// Generate additional parts of the dungeon
			GenerateDungeon(DefaultFloorClass, DefaultWallClass, DefaultWallClassLightned, DefaultEntranceClass, DefaultRoofClass, RoomInitial);
		}
	}
}

// Generates additional parts of the dungeon
void ASpawnDungeon::GenerateDungeon(TSubclassOf<AActor> FloorClass, TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, TSubclassOf<AActor> EntranceClass, TSubclassOf<AActor> RoofClass, ASpawnRoom* RoomOfOrigin){
	UWorld* World = GetWorld();

	if (!World or !RoomOfOrigin) return;

	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Generating dungeon from room ID: %d"), RoomOfOrigin->GetParamRoomID()));

	// Set corridors connected to the original room
	for (AActor* Entrance : RoomOfOrigin->RoomObjects.EntranceObject) {
		// Spawn a new corridor
		ASpawnCorridor* Corridor = World->SpawnActor<ASpawnCorridor>(ASpawnCorridor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

		if (Corridor) {
			SetCorridorParameters(Corridor, Entrance);
			Corridor->CreateCorridor();
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Corridor created at location: %s"), *Corridor->GetActorLocation().ToString()));
		}
		CorridorDungeon.Add(Corridor);
	}

	// Set rooms connected to the corridors
	TArray<FEntranceStruct> EntranceInfo;

	// Ensure the number of corridors matches the number of entrances
	check(CorridorDungeon.Num() == RoomOfOrigin->RoomObjects.EntranceObject.Num());

	for (int32 Index = 0; Index < CorridorDungeon.Num(); Index++) {
		ASpawnCorridor* Corridor = CorridorDungeon[Index];
		AActor* Entrance = RoomOfOrigin->RoomObjects.EntranceObject[Index];
		if (Corridor and Entrance) {
			FVector Direction = GetEntranceDirection(Entrance->GetActorRotation());
			float OffsetDistance = GetOffsetDistance(Corridor);
			FVector NewLocation = GetNewLocation(Entrance->GetActorLocation(), Direction, OffsetDistance);

			// Create an entrance structure
			FEntranceStruct NewEntrance = FEntranceStruct(EntranceClass, -1, true, NewLocation, Entrance->GetActorRotation());

			// Spawn a new room
			ASpawnRoom* Room = World->SpawnActor<ASpawnRoom>(ASpawnRoom::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

			float OriginalYaw = NewEntrance.EntranceRotation.Yaw;
			int32 RandomRangeValue = GetRandomRangeValue(Room, OriginalYaw);

			NewEntrance.EntranceID = RandomRangeValue;

			float NewYaw = GetNewYaw(OriginalYaw);
			NewEntrance.EntranceRotation = FRotator(0.f, NewYaw, 0.f);

			EntranceInfo.Add(NewEntrance);

			if (Room) {
				FVector AdjustVector = GetAdjustVector(Room, NewEntrance);
				Room->SetParamStartLocation(NewEntrance.EntrancePosition - AdjustVector);
				Room->CreateRoom(NewEntrance);
				RoomDungeon.Add(Room);
				ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Room created with ID: %d"), Room->GetParamRoomID()));
			}
		}
	}
}

// Continuously checks and regenerates the dungeon when the player moves to a new room
void ASpawnDungeon::GenerateDungeonEternal(){
	UWorld* World = GetWorld();
	if (!World) return;
	APawn* PlayerPawn = GetPlayePawn();
	
	if (!PlayerPawn) return;
	ASpawnRoom* CurrentRoom = GetCurrentRoom(PlayerPawn);
	
	if (!CurrentRoom or CurrentRoom->GetParamRoomID() == CurrentRoomID) return;

	AsyncTask(ENamedThreads::GameThread, [this, CurrentRoom]() {
		ClearDungeon(CurrentRoom);
		});

	TSubclassOf<AActor> FloorClass = DefaultFloorClass, WallClass = DefaultWallClass, WallClassLightned = DefaultWallClassLightned, EntranceClass = DefaultEntranceClass, RoofClass = DefaultRoofClass;

	// Generate dungeon asynchronously
	AsyncTask(ENamedThreads::GameThread, [this, FloorClass, WallClass, WallClassLightned, EntranceClass, RoofClass, CurrentRoom]() {
		GenerateDungeon(FloorClass, WallClass, WallClassLightned, EntranceClass, RoofClass, CurrentRoom);
		});
	
	CurrentRoomID = CurrentRoom->GetParamRoomID();
	ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Player moved to new room ID: %d"), CurrentRoomID));
}

void ASpawnDungeon::SetCorridorParameters(ASpawnCorridor* Corridor, AActor* Entrance) {
	const float Tolerance = 0.1f;
	float Yaw = Entrance->GetActorRotation().Yaw;

	const int32 RandomWalls = FMath::FRandRange(3.f, 12.f);
	const float WallLength = Corridor->GetParamWallLength();
	const FVector EntranceLocation = Entrance->GetActorLocation();

	struct FCorridorConfig {
		int32 ForwardWalls;
		int32 RightWalls;
		FVector Offset;
	};

	// Mapping of Yaw values to corridor configurations
	TMap<float, FCorridorConfig> YawConfigMap = {
		{ 0.f  , {1, RandomWalls, FVector(0.f, WallLength * RandomWalls, 0.f)}},
		{ 90.f , {RandomWalls, 1, FVector(-WallLength / 2, WallLength / 2, 0.f)}},
		{ 180.f, {1, RandomWalls, FVector::ZeroVector}},
		{ -90.f, {RandomWalls, 1, FVector(WallLength * RandomWalls - WallLength / 2, WallLength / 2, 0.f)}}
	};

	const bool bFoundConfig = false;

	// Apply the appropriate corridor configuration based on the Yaw value
	for (const auto& Entry : YawConfigMap) {
		if (FMath::IsNearlyEqual(Yaw, Entry.Key, Tolerance)) {
			const FCorridorConfig& Config = Entry.Value;
			Corridor->SetParamForwardWalls(Config.ForwardWalls);
			Corridor->SetParamRightWalls(Config.RightWalls);
			Corridor->SetParamStartLocation(EntranceLocation - Config.Offset);
			ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Corridor parameters set for Yaw: %f"), Yaw));
			return;
		}
	}

	if (!bFoundConfig) {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oops, something went wrong. Unexpected Yaw: %f"), Yaw), FColor::Red);
	}
}

// Method to get entrance direction
FVector ASpawnDungeon::GetEntranceDirection(FRotator Rotation) {
	return FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
}

// Method to get offset distance based on corridor parameters
float ASpawnDungeon::GetOffsetDistance(ASpawnCorridor* Corridor) {
	return FMath::Max(Corridor->GetParamForwardWalls() * Corridor->GetParamWallLength(), Corridor->GetParamRightWalls() * Corridor->GetParamWallLength());
}

// Method to get new location
FVector ASpawnDungeon::GetNewLocation(FVector EntranceLocation, FVector Direction, float OffsetDistance) {
	return EntranceLocation - Direction * OffsetDistance;
}

// Method to get random range value based on yaw
int32 ASpawnDungeon::GetRandomRangeValue(ASpawnRoom * Room, float OriginalYaw) {
	int32 RandomRangeValue = 0;
	float AdjustedYaw = FMath::Fmod(FMath::Abs(OriginalYaw), 360.f);

	TMap<float, int32> RandomRangeMap = {
		{0.f, Room->GetParamForwardWalls() - FMath::RandRange(1, Room->GetParamForwardWalls() - 1)},
		{180.f, Room->GetParamForwardWalls() - FMath::RandRange(1, Room->GetParamForwardWalls() - 1)},
		{90.f, Room->GetParamRightWalls() - FMath::RandRange(1, Room->GetParamRightWalls() - 1)},
		{-90.f, Room->GetParamRightWalls() - FMath::RandRange(1, Room->GetParamRightWalls() - 1)}
	};

	if (RandomRangeMap.Contains(AdjustedYaw)) {
		RandomRangeValue = RandomRangeMap[AdjustedYaw];
	}
	else {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oops, something went wrong. Unexpected original yaw angle: %f"), OriginalYaw), FColor::Red);
	}

	return RandomRangeValue;
}

// Method to get new yaw based on original yaw
float ASpawnDungeon::GetNewYaw(float OriginalYaw) {
	TMap<float, float> YawAngleMap = {
		{0.f, 180.f},
		{90.f, -90.f},
		{180.f, 0.f},
		{-90.f, 90.f}
	};

	if (YawAngleMap.Contains(OriginalYaw)) {
		return YawAngleMap[OriginalYaw];
	}
	else {
		ULoggingTool::LogDebugMessage(FString::Printf(TEXT("Oops, something went wrong. Unexpected original yaw angle: %f"), OriginalYaw), FColor::Red);
		return 0.f;
	}
}

// Method to get adjustment vector based on yaw and entrance ID
FVector ASpawnDungeon::GetAdjustVector(ASpawnRoom* Room, FEntranceStruct NewEntrance) {
	TMap<float, FVector> VectorConfigMap = {
		{0.f, FVector((Room->GetParamForwardWalls() - NewEntrance.EntranceID - 1) * Room->GetParamWallLength(), 0.f, 0.f)},
		{90.f, FVector(Room->GetParamForwardWalls() * Room->GetParamWallLength() - Room->GetParamWallLength() / 2, NewEntrance.EntranceID * Room->GetParamWallLength() + Room->GetParamWallLength() / 2, 0.f)},
		{180.f, FVector((Room->GetParamForwardWalls() - NewEntrance.EntranceID - 1) * Room->GetParamWallLength(), Room->GetParamRightWalls() * Room->GetParamWallLength(), 0.f)},
		{-90.f, FVector(-Room->GetParamWallLength() / 2, (Room->GetParamRightWalls() - NewEntrance.EntranceID) * Room->GetParamWallLength() - Room->GetParamWallLength() / 2, 0.f)}
	};

	if (VectorConfigMap.Contains(NewEntrance.EntranceRotation.Yaw)) {
		return VectorConfigMap[NewEntrance.EntranceRotation.Yaw];
	}
	else {
		return FVector::ZeroVector;
	}
}

// Method to get Player's pawn
APawn* ASpawnDungeon::GetPlayePawn() const{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	return PlayerController ? PlayerController->GetPawn() : nullptr;
}

// Method to get current room
ASpawnRoom* ASpawnDungeon::GetCurrentRoom(APawn* PlayerPawn){
	for (ASpawnRoom* Room : RoomDungeon) {
		if (Room && Room->PlayerDetectBox->IsOverlappingActor(PlayerPawn)) {
			return Room;
		}
	}
	return nullptr;
}

// Method to clear dungeon rooms
void ASpawnDungeon::ClearDungeon(ASpawnRoom* RoomToSkip) {
	// Ensure this runs on the game thread
	if (!IsInGameThread()) {
		AsyncTask(ENamedThreads::GameThread, [this, RoomToSkip]() {
			ClearDungeon(RoomToSkip);
			});
		return;
	}

	UWorld* World = GetWorld();
	if (World) {
		for (ASpawnRoom* Room : RoomDungeon) {
			if (Room != RoomToSkip and Room) {
				Room->DestroyRoom();
				Room->Destroy();
			}
		}
		RoomDungeon.Empty();
		RoomDungeon.Add(RoomToSkip);

		// Clear corridors if any
		ClearCorridors();
	}
}

// Method to clear corridors
void ASpawnDungeon::ClearCorridors() {
	if (!IsInGameThread()) {
		AsyncTask(ENamedThreads::GameThread, [this]() {
			ClearCorridors();
			});
		return;
	}
	UWorld* World = GetWorld();
	if (World) {
		for (ASpawnCorridor* Corridor : CorridorDungeon) {
			if (Corridor) {
				Corridor->DestroyCorridor();
				Corridor->Destroy();
			}
		}
		CorridorDungeon.Empty();
	}
}