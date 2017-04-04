// SM

#pragma once

#include "GameFramework/Actor.h"
#include "Wall.generated.h"

UCLASS()
class AP_ASSIGNMENT_API AWall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWall();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
		UBoxComponent* collisionComponent;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* staticMeshComponent;

	UPROPERTY(EditAnywhere)
		float rad;	//SweepResult doesn't show anything!!! So, Sweep trace done to get hit & normal


	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
};

