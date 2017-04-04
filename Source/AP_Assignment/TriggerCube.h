// SM

#pragma once

#include "GameFramework/Actor.h"
#include "TriggerCube.generated.h"

UCLASS()
class AP_ASSIGNMENT_API ATriggerCube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATriggerCube();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
		UBoxComponent* collisionComponent;
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void CreateNewPhysicsConstraintBetween(AStaticMeshActor* RootSMA, AStaticMeshActor* TargetSMA);	//

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Constraints)
	//	UPhysicsConstraintComponent* axleBoardConstraint;
	//UPROPERTY(VisibleAnywhere)
	//	UStaticMeshComponent* staticMeshComponent;
	//UPROPERTY(VisibleAnywhere)
	//	FName Constraint_ComponentName_1;
	//UPROPERTY(VisibleAnywhere)
	//	FName Constraint_ComponentName_2;


	UPROPERTY(EditAnywhere, Category = LaunchBoard)
		float rotSpeed;
	UPROPERTY(EditAnywhere, Category = LaunchBoard)
		FVector rotPt;

	AStaticMeshActor *constraint1, *constraint2;
	void setNewRotation(AStaticMeshActor *actorToRotate, float rotAng, bool interp);
	FRotator targRot;

	UPROPERTY(EditAnywhere)
		FVector angImpulse;
};


