#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DataStructures/EntranceStruct.h"
#include "DataStructures/RoomStruct.h"
#include "DataStructures/RoomAssetStruct.h"
#include "SpawnRoom.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpawnRoom, Log, All)

UCLASS() class GAMEDEMO_API ASpawnRoom : public AActor {
	GENERATED_BODY()
	
public:	
	ASpawnRoom();
	~ASpawnRoom();

	UFUNCTION(BlueprintCallable, Category = "Overloaded Constructor")
	void Init(int32 ParamForwardWalls, int32 ParamRightWalls, float ParamWallLength, FVector ParamStartLocation, FRotator ParamStartRotation);
	
	UFUNCTION(BlueprintCallable, Category = "Overloaded Constructor")
	void InitByClass(ASpawnRoom* Class);

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Struct Room")
	FRoomStruct RoomObjects;

protected:
	virtual void BeginPlay() override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Spawn Floor")
	void CreateFloor(TSubclassOf<AActor> FloorClass, FVector StartLocation, FRotator StartRotation, int32 FloorWidth, int32 FloorLength, float WallLength);

	UFUNCTION(BlueprintCallable, Category = "Spawn Wall")
	void CreateWall(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, TSubclassOf<AActor> EntranceClass, FVector StartLocation, FRotator StartRotation, int32 NumberOfWallsForward, int32 NumberOfWallsRight, float WallLength, float GapBetweenWalls, FEntranceStruct EntranceObject = FEntranceStruct());

	UFUNCTION(BlueprintCallable, Category = "Spawn Roof")
	void CreateRoof(TSubclassOf<AActor> RoofClass, FVector StartLocation, FRotator StartRotation, int32 RoofWidth, int32 RoofLength, float WallLength);

	UFUNCTION(BlueprintCallable, Category = "Spawn Room")
	void InitAssets();
public:
	UFUNCTION(BlueprintCallable, Category = "Spawn Room")
	void CreateRoom(FEntranceStruct EntranceInfo = FEntranceStruct());
	
	UFUNCTION(BlueprintCallable, Category = "Player Detect Comp")
	void CreatePlayerDetector();

	UPROPERTY(EditAnywhere, Category = "Player Detect Comp")
	UBoxComponent* PlayerDetectBox;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Destroy Room")
	void DestroyRoom();

private:	
	UFUNCTION(BlueprintCallable, Category = "Spawn Wal")
	FVector DotProduct(FVector PositioningVector, FVector SymbolVector, float WallLength, float WallHeight);
	
	UFUNCTION(BlueprintCallable, Category = "Spawn Wall")
	FVector GenerateWalls(UWorld* World, TSubclassOf<AActor> WallClass, FVector StartLocation = FVector::ZeroVector, FRotator StartRotation = FRotator::ZeroRotator, int32 NumberOfWalls = -1, float WallLength = -1.f, float GapBetweenWalls = -1.f, FEntranceStruct EntranceObject = FEntranceStruct(), TSubclassOf<AActor> WallClassLightned = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Spawn Wall")
	FVector CreateWallSegment(TSubclassOf<AActor> WallClass, TSubclassOf<AActor> WallClassLightned, FEntranceStruct& EntranceObject, FVector StartLocation, FRotator StartRotation, int32 NumberOfWalls, float WallLength, float GapBetweenWalls);

	void CreateInitialDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateCombatDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreatePuzzleDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateRiddleDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateSecretDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateTreasureDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateMerchantDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateBountyDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateCauldronDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateTavernDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateChillDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateCoolGuyDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreatePortalDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateEmptyDeco(FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	UFUNCTION(BlueprintCallable, Category = "Spawn Entrance")
	bool bWillBeEntrance();

	UFUNCTION(BlueprintCallable, Category = "Spawn Entrance")
	FEntranceStruct GenerateEntranceStruct(float NumberOfWalls, TSubclassOf<AActor> EntranceClass, FEntranceStruct EntranceStruct);

	FEntranceStruct GenerateEntranceStruct(FVector Position, TSubclassOf<AActor> EntranceClass, FEntranceStruct EntranceStruct);

	//START: Debug output section

	UFUNCTION(BlueprintCallable, Category = "Debug Output") 
	void OutputEntrancePosition() { RoomObjects.EntranceObjectOutput(); }

	UFUNCTION(BlueprintCallable, Category = "Debug Output")
	void OutputFloorPosition(){ RoomObjects.FloorObjectOutput(); }

	UFUNCTION(BlueprintCallable, Category = "Debug Output")
	void OutputWallPosition() { RoomObjects.WallObjectOutput(); }

	UFUNCTION(BlueprintCallable, Category = "Debug Output")
	void OutputRoofPosition() { RoomObjects.RoofObjectOutput(); }
	
	//END  : Debug output section 

	FString RoomTag;

	void AssignRoomTag(const FString& FilePath);

	void AssignRoomAssets(const FString& FilePath);

	static int32 RoomCount;
	int32 RoomID;
	bool bIsPlayerInRoom;

	FTimerHandle PlayerCheckTimerHandle;
	AActor* PlayerActor;

	void CheckPlayerPosition();
	bool IsPointInsideBox(const FVector &Point, const FVector &BoxCenter, const FVector &BoxExtent, float VerticalMargin);

	UClass* LoadAssetClass(const FString& AssetPath);

	void CreateMainPillars(UClass* PillarClass, FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateSupportivePillars(UClass* PillarClass, FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateBoxes(UClass* BoxClass, FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateChairs(UClass* ChairClass, FVector StartLocation, float WallLength, int32 NumberOfWallsForward, int32 NumberOfWallsRight);

	void CreateSingleProp(UClass* PropClass, FVector StartLocation = FVector::ZeroVector, FRotator StartRotation = FRotator::ZeroRotator);

	int32 ParamForwardWalls;

	int32 ParamRightWalls;

	float ParamWallLength;

	FVector ParamStartLocation;

	FRotator ParamStartRotation;

	float EntranceProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Assets", meta = (AllowPrivateAccess = "true"))
	TArray<FAssetStruct> RoomAssetData;

	TSubclassOf<AActor> DefaultFloorClass;
	TSubclassOf<AActor> DefaultWallClass;
	TSubclassOf<AActor> DefaultWallClassLightned;
	TSubclassOf<AActor> DefaultEntranceClass;
	TSubclassOf<AActor> DefaultRoofClass;

public:
	//START: Getters

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	int32 GetParamForwardWalls() const { return ParamForwardWalls; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	int32 GetParamRightWalls() const { return ParamRightWalls; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	float GetParamWallLength() const { return ParamWallLength; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	FVector GetParamStartLocation() const {	return ParamStartLocation; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	FRotator GetParamStartRotation() const { return ParamStartRotation; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	int32 GetParamRoomID() const { return RoomID; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	bool GetParamIsPlayerInRoom() const { return bIsPlayerInRoom; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	FString GetParamRoomTag() const { return RoomTag; }
	//END  : Getters

	//START: Setters
	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamForwardWalls(int32 NewValue) { this->ParamForwardWalls = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamRightWalls(int32 NewValue) { this->ParamRightWalls = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamWallLength(float NewValue) { this->ParamWallLength = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamStartLocation(FVector NewValue) { this->ParamStartLocation = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamStartRotation(FRotator NewValue) { this->ParamStartRotation = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamRoomID(int32 NewValue) { this->RoomID = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Struct Room")
	void SetParamRoomTag(FString NewValue) { RoomTag = NewValue; }
	//END  : Setters

private:
	bool IsLocationValid(FVector Location, FVector RoomMin, FVector RoomMax, float WallLength);
};
