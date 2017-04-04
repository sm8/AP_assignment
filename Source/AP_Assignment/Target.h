// SM

#pragma once

#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"	
#include "Perception/AISenseConfig_Sight.h"
#include "Target.generated.h"

UCLASS()
class AP_ASSIGNMENT_API ATarget : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATarget();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category = Target)
		UCapsuleComponent* collisionComponent;// capsule collision component.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		bool isMoving;

	UPROPERTY(VisibleDefaultsOnly, Category = Target)
		UAIPerceptionComponent *aiPercComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Target)
		UAISenseConfig_Sight *sightConfig;

	UFUNCTION()
		void OnSensed(TArray<AActor*> testActors);

	UPROPERTY(VisibleAnywhere, Category = Movement)
		FVector baseLoc;
	UPROPERTY(VisibleAnywhere, Category = Movement)
		FVector currVel;
	UPROPERTY(VisibleAnywhere, Category = Movement)
		float speed;

	void setNewRotation(AActor *actorToRotate, FVector targPos, FVector currPos);	//for rotations

	UPROPERTY(VisibleAnywhere, Category = Movement)
		FRotator targRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float rotationSpeed;
	bool interp;
};







