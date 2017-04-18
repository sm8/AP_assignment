// SM

#pragma once

#include "GameFramework/Actor.h"
#include "CreateHeatmap.generated.h"

UCLASS()
class AP_ASSIGNMENT_API ACreateHeatmap : public AActor {
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
	UPROPERTY(EditAnywhere)
		unsigned int widthOfHeatmapInPixels;
	UPROPERTY(EditAnywhere)
		unsigned int heightOfHeatmapInPixels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerMovementColour)
		FColor playerMoveCol;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticObjectColour)
		FColor staticObjCol;

	float totTime, prevTime, maxTimeAtPos;	//for Heatmap 
	struct PosData {
		float x, y, z, dt;
		PosData(float nx, float ny, float nz, float ndt) { x = nx; y = ny; z = nz; dt = ndt; }
	};
	TArray<PosData> playerPos;
	TArray<FString> playerPositions;
	FString allPlayerPos;
	float maxX, maxY, minX, minY, playerRad, chkSqVal;
	unsigned int w, h;
	struct ActorAndBounds {
		AActor *actor;
		FVector maxBounds, org;
		ActorAndBounds(AActor* a, FVector o, FVector mb) { actor = a; org = o; maxBounds = mb; }
	};
	TArray<ActorAndBounds*> staticActors;
	void SaveTexture2DDebug(const uint8* PPixelData, int width, int height, FString Filename);
	void addStaticBoundsToHeatmap(uint8 *pixels, float gx, float gy);
	void set32BitPixel(uint8 *pixels, int pos, uint8 r, uint8 g, uint8 b, uint8 a);
	void outputArrayCSVfile(int w, int h, uint8 *pixels, FString filename);
	void updateLastPositionInArrays();
	void updatePositionData(FVector &newLocation);
	void addPlayerPosToTextArray(FVector & newLocation, float diffT);
	unsigned int getGridPos(float rx, float minX, float gx);
	void saveHeatmap();
	void clearArrays(uint8 * pixels);
	bool checkBindingIsSetup(FString axisMapName);
	float calcDiffInTime();

	FVector prevLoc;
	APawn *player;	//Pawn needed as BindAxis used
	AActor *platform;
	bool heatMapProcessed;
	const int BPP = 4; //Bytes per pixel for PNG output
};
