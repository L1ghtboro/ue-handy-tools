#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnRoom.h"
#include "SpawnCorridor.h"
#include "SpawnDungeon.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpawnDungeon, Log, All)

UCLASS() class GAMEDEMO_API ASpawnDungeon : public AActor {
	GENERATED_BODY()
	
public:	
	ASpawnDungeon();

	TArray<ASpawnRoom*> RoomDungeon;
	TArray<ASpawnCorridor*> CorridorDungeon;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void InitAssets(TSubclassOf<AActor> FloorClass, TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, TSubclassOf<AActor> EntranceClass, TSubclassOf<AActor> RoofClass);

	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void GenerateDungeonOnBoot();

	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void GenerateDungeon(TSubclassOf<AActor> FloorClass, TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, TSubclassOf<AActor> EntranceClass, TSubclassOf<AActor> RoofClass, ASpawnRoom* RoomOfOrigin);

	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void GenerateDungeonEternal();

	void ClearDungeon(ASpawnRoom* RoomToSkip);

private:
	void SetCorridorParameters(ASpawnCorridor* Corridor, AActor* Entrance);
	
	FVector GetEntranceDirection(FRotator Rotation);

	float GetOffsetDistance(ASpawnCorridor* Corridor);

	FVector GetNewLocation(FVector EntranceLocation, FVector Direction, float OffsetDistance);

	int32 GetRandomRangeValue(ASpawnRoom* Room, float OriginalYaw);

	float GetNewYaw(float OriginalYaw);

	FVector GetAdjustVector(ASpawnRoom* Room, FEntranceStruct NewEntrance);

	APawn* GetPlayePawn() const;

	ASpawnRoom* GetCurrentRoom(APawn* PlayerPawn);

	void ClearCorridors();

	int32 CurrentRoomID;

	int32 RoomSpawned;

	FTimerHandle DungeonCheckTimerHandle;

	TSubclassOf<AActor> DefaultFloorClass;
	TSubclassOf<AActor> DefaultWallClass;
	TSubclassOf<AActor> DefaultWallClassLightned;
	TSubclassOf<AActor> DefaultEntranceClass;
	TSubclassOf<AActor> DefaultRoofClass;
};