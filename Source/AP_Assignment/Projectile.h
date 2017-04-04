// SM

#pragma once

#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class AP_ASSIGNMENT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void FireInDirection(const FVector &shootDirection);

	// Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* collisionComponent;// Sphere collision component.

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	//UStaticMeshComponent* staticMeshComponent;

	// Projectile movement component.
	UPROPERTY(VisibleAnywhere, Category = Movement)
		UProjectileMovementComponent* projectileMovementComponent;

	// sphere radius from the Owner pawn's location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float initRad;

	// init speed.
	UPROPERTY(EditAnywhere, Category = Gameplay)
		float initSpeed;
	
	void createCollisionComp();
	void createProjMovementComp();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere, Category = Gameplay)
		UMaterial* material1;

	static int numOfHits;
};







