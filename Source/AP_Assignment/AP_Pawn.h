// SM

#pragma once

#include "Projectile.h"
#include "Target.h"
#include "GameFramework/Pawn.h"
#include "AP_Pawn.generated.h"

UCLASS()
class AP_ASSIGNMENT_API AAP_Pawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAP_Pawn();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void updatePositionData(FVector &newLocation);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(EditAnywhere)
		UCapsuleComponent* collisionComponent;

	UPROPERTY(EditAnywhere)
		UCameraComponent* camera;

	// offset from the Owner pawn's location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector projOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool isMovingFwd;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool isMovingRgt;

	// Projectile class to spawn.
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AProjectile> ProjectileClass;

	// Target class to spawn.
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class ATarget> TargetClass;

	//Input functions
	void rotatePawn(float r);
	void updateLastPositionInArrays();
	void moveX(float x);
	void moveY(float y);

	//Input variables
	FVector currVel;
	bool isGrowing;
	const float SPEED = 100.0f;
	float angle;

	//Mouse Input functions
	void turn(float t);
	void lookUp(float l);

	void fire();

	const int NUM_TARGETS = 6;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UPROPERTY(EditAnywhere)
		float rad;	//SweepResult doesn't show anything!!! So, Sweep trace done to get hit & normal

	FRotator targRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float rotationSpeed;
	bool interp;

	float totTime, prevTime, maxTimeAtPos;	//for Heatmap 
	struct PosData {
		float x, y, z, dt;
		PosData(float nx, float ny, float nz, float ndt) { x = nx; y = ny; z = nz; dt = ndt; }
	};
	TArray<PosData> pawnPs;
	TArray<FString> pawnPositions;
	FString allPawnPos;
	void outputArrayCSVfile(int w, int h, uint8 *pixels, FString filename);
	unsigned int getGridPos(float rx, float minX, float gx);
	float maxX, maxY, minX, minY, pawnRad;
	int w, h;
	struct ActorAndBounds {
		AActor *actor;
		FVector maxBounds, org;
		ActorAndBounds(AActor* a, FVector o, FVector mb) { actor = a; org = o; maxBounds = mb; }
	};
	TArray<ActorAndBounds*> staticActors;
	void addStaticBoundsToHeatmap(uint8 *pixels, float gx, float gy);
};
