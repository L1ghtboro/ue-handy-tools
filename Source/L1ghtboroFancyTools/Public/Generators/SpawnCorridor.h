#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataStructures/CorridorStruct.h"
#include "DataStructures/RoomAssetStruct.h"
#include "SpawnCorridor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpawnCorridor, Log, All)

UCLASS() class GAMEDEMO_API ASpawnCorridor : public AActor {
	GENERATED_BODY()
	
public:	
	ASpawnCorridor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Struct Corridor")
	FCorridorStruct CorridorObjects;

	void InitAssets();

protected:
	virtual void BeginPlay() override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Spawn Floor")
	void CreateFloor(TSubclassOf<AActor> FloorClass, FVector StartLocation, FRotator StartRotation, int32 FloorLength, int32 FloorWidth, float WallLength);

	UFUNCTION(BlueprintCallable, Category = "Spawn Wall")
	void CreateWall(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLighted, FVector StartLocation, FRotator StartRotation, int32 NumberOfWallsForward, int32 NumberOfWallsRight, float WallLength, float GapBetweenWalls);

	UFUNCTION(BlueprintCallable, Category = "Spawn Roof")
	void CreateRoof(TSubclassOf<AActor> RoofClass, FVector StartLocation, FRotator StartRotation, int32 RoofLength, int32 RoofWidth, float WallLength, float WallHeight, int32 WallRows);
public:
	UFUNCTION(BlueprintCallable, Category = "Spawn Roof")
	void CreateCorridor();

	UFUNCTION(BlueprintCallable, Category = "Spawn Roof")
	void DestroyCorridor();

	virtual void Tick(float DeltaTime) override;

private: 
	UFUNCTION(BlueprintAuthorityOnly, Category = "Spawn Wall")
	FVector CreateWallSegment(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLighted, FVector StartLocation, FRotator StartRotation, int32 NumberOfWalls, float WallLength, float GapBetweenWalls);

	UFUNCTION(BlueprintAuthorityOnly, Category = "Spawn Wall")
	FVector GenerateWalls(UWorld* World, TSubclassOf<AActor> WallClass, FVector StartLocation = FVector::ZeroVector, FRotator StartRotation = FRotator::ZeroRotator, int32 NumberOfWalls = -1, float WallLength = -1.f, float GapBetweenWalls = -1.f, TSubclassOf<AActor> WallClassLightned = nullptr);

	FVector DotProduct(FVector PositioningVector, FVector SymbolVector, float WallLength, float WallHeight);

	void AssignCorridorAssets(const FString& FilePath);

	int32 ParamForwardWalls;

	int32 ParamRightWalls;

	float ParamWallLength;

	FVector ParamStartLocation;

	FRotator ParamStartRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Assets", meta = (AllowPrivateAccess = "true"))
	TArray<FAssetStruct> CorridorAssetData;

	FString CorridroTag;

	TSubclassOf<AActor> DefaultFloorClass;
	TSubclassOf<AActor> DefaultWallClass;
	TSubclassOf<AActor> DefaultWallClassLightned;
	TSubclassOf<AActor> DefaultRoofClass;
public:
	//Getters
	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	int32 GetParamForwardWalls() const { return ParamForwardWalls; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	int32 GetParamRightWalls() const { return ParamRightWalls; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	float GetParamWallLength() const { return ParamWallLength; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	FVector GetParamStartLocation() const { return ParamStartLocation; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	FRotator GetParamStartRotation() const { return ParamStartRotation; }

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	void SetParamForwardWalls(int32 NewValue) { this->ParamForwardWalls = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	void SetParamRightWalls(int32 NewValue) { this->ParamRightWalls = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	void SetParamWallLength(float NewValue) { this->ParamWallLength = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	void SetParamStartLocation(FVector NewValue) { this->ParamStartLocation = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Corridor")
	void SetParamStartRotation(FRotator NewValue) { this->ParamStartRotation = NewValue; }
};
