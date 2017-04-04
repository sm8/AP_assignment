// SM

#include "AP_Assignment.h"
#include "Projectile.h"
#include "Target.h"

int AProjectile::numOfHits = 0;

// Sets default values
AProjectile::AProjectile(){
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	initSpeed = 2000.0f;

	createCollisionComp();
	createProjMovementComp();
}

void AProjectile::createCollisionComp() {
	// Use a sphere as a simple collision representation.
	collisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	initRad = 15.0f;
	// Set the sphere's collision radius.
	collisionComponent->InitSphereRadius(initRad);
	// Set the root component to be the collision component.
	collisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	collisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	RootComponent = collisionComponent;
}
void AProjectile::createProjMovementComp(){
	// Use this component to drive this projectile's movement.
	projectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	projectileMovementComponent->SetUpdatedComponent(collisionComponent);
	projectileMovementComponent->InitialSpeed = initSpeed;
	projectileMovementComponent->MaxSpeed = 3000.0f;
	projectileMovementComponent->bRotationFollowsVelocity = true;
	projectileMovementComponent->bShouldBounce = true;
	projectileMovementComponent->Bounciness = 0.3f;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay(){
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );

}

void AProjectile::FireInDirection(const FVector & ShootDirection) {
	projectileMovementComponent->Velocity = ShootDirection * projectileMovementComponent->InitialSpeed;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit){
	if (OtherActor != this && OtherComp->IsSimulatingPhysics())
		OtherComp->AddImpulseAtLocation(projectileMovementComponent->Velocity * 100.0f, Hit.ImpactPoint);
	if (OtherActor != NULL && OtherActor->GetName().Contains("Target")) {
		ATarget* targ = ((ATarget*)OtherActor);	//cast to Target
		if (targ) {
			//TArray<UStaticMeshComponent*> components;	
			//targ->GetComponents<UStaticMeshComponent>(components);	//get static comps
			//components[0]->SetMaterial(0, material1);	//we only have 1 static mesh, so use 0

			numOfHits++;
			FString s = TEXT("Hits: ") + FString::FromInt(numOfHits);
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, s);
			UE_LOG(LogTemp, Warning, TEXT("Target %s Hits: %d"), *OtherActor->GetName(), numOfHits);
		}
	}
}






