// SM

#pragma once

#include "GameFramework/Pawn.h"
#include "CreateHeatmap.generated.h"

UCLASS()
class AP_ASSIGNMENT_API ACreateHeatmap : public APawn{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACreateHeatmap();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
		FString nameOfPlatform;
	UPROPERTY(EditAnywhere)
		FString nameOfPlayerToTrack;

	float totTime, prevTime, maxTimeAtPos;	//for Heatmap 
	struct PosData {
		float x, y, z, dt;
		PosData(float nx, float ny, float nz, float ndt) { x = nx; y = ny; z = nz; dt = ndt; }
	};
	TArray<PosData> pawnPs;
	TArray<FString> pawnPositions;
	FString allPawnPos;
	float maxX, maxY, minX, minY, pawnRad;
	int w, h;
	struct ActorAndBounds {
		AActor *actor;
		FVector maxBounds, org;
		ActorAndBounds(AActor* a, FVector o, FVector mb) { actor = a; org = o; maxBounds = mb; }
	};
	TArray<ActorAndBounds*> staticActors;
	void SaveTexture2DDebug(const uint8* PPixelData, int width, int height, FString Filename);
	void addStaticBoundsToHeatmap(uint8 *pixels, float gx, float gy);
	void outputArrayCSVfile(int w, int h, uint8 *pixels, FString filename);
	void updateLastPositionInArrays();
	void updatePositionData(FVector &newLocation);
	unsigned int getGridPos(float rx, float minX, float gx);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	void saveHeatmap();
	FVector prevLoc;
	APawn *player;
	bool heatMapProcessed;
};
