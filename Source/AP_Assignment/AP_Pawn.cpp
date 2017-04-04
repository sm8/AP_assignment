// SM

#include "AP_Assignment.h"
#include "AP_Pawn.h"
#include "Target.h"
#include "Perception/AISense_Sight.h"	//used to register this pawn
#include "Wall.h"	//****

// Sets default values
AAP_Pawn::AAP_Pawn(){
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create a dummy root component we can attach things to.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	// Create a camera and a visible object
	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("camera"));
	collisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("pawnColComponent"));
	// Set the capsule's collision radius and half-height.
	collisionComponent->SetCapsuleSize(30.0f, 100.0f);
	collisionComponent->SetupAttachment(RootComponent);

	collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AAP_Pawn::OnBeginOverlap);
	collisionComponent->OnComponentEndOverlap.AddDynamic(this, &AAP_Pawn::OnOverlapEnd);

	// Attach camera & visible object to root component. Offset & rotate camera.
	camera->SetupAttachment(RootComponent);
	camera->SetRelativeLocation(FVector(-450.0f, 0.0f, 350.0f));
	camera->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));

	isMovingFwd = false;	//****
	isMovingRgt = false;
	rad = 100.0f;
	interp = false;
	rotationSpeed = 1.0f;
}

// Called when the game starts or when spawned
void AAP_Pawn::BeginPlay(){
	Super::BeginPlay();

	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), this);

	AProjectile::numOfHits = 0;	//stops static being reused when game restarted.
	targRot = GetActorRotation();	//****
	UWorld* World = GetWorld();
	if (World) {	// Spawn targets
		FVector targLoc;
		FRotator projRot = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;
		float ang = FMath::DegreesToRadians(360.0f / ((float)NUM_TARGETS));
		float targetPosRadius = 500.0f;
		for (int i = 0; i < NUM_TARGETS; i++) {
			targLoc = GetActorLocation() + FVector(targetPosRadius*FMath::Cos((float)i*ang), targetPosRadius*FMath::Sin((float)i*ang), 0.0f);
			ATarget *targ = World->SpawnActor<ATarget>(TargetClass, targLoc, projRot, SpawnParams);
			targ->baseLoc = targLoc;
		}
	}
}

// Called every frame
void AAP_Pawn::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );

	if (interp) {	//interpolate 
		FRotator nr = FMath::RInterpTo(GetActorForwardVector().Rotation(), targRot, DeltaTime, rotationSpeed);
		nr.Pitch = nr.Roll = 0.0f;
		if ((nr - targRot).IsNearlyZero(0.5f)) {
			nr = targRot;
			interp = false;
		}
		SetActorRotation(nr);
	}

	// Handle movement based on "MoveX" and "MoveY" axes
	if (!currVel.IsZero()) {
		FVector newLocation = GetActorLocation() + currVel * DeltaTime;
		SetActorLocation(newLocation);
	}
	// Handle angular movement based on Rotate
	if (angle != 0.0f) {
		FRotator newRot = GetActorRotation();
		newRot.Yaw += angle;	//rotate about Z axis
		SetActorRotation(newRot);
	}
}

// Called to bind functionality to input
void AAP_Pawn::SetupPlayerInputComponent(class UInputComponent* InputComponent){
	Super::SetupPlayerInputComponent(InputComponent);
	// Respond when our "Grow" key is pressed or released.
	InputComponent->BindAction("Fire", IE_Released, this, &AAP_Pawn::fire);

	// Respond each frame to: "MoveX" &"MoveY" and Rotate.
	InputComponent->BindAxis("MoveX", this, &AAP_Pawn::moveX);
	InputComponent->BindAxis("MoveY", this, &AAP_Pawn::moveY);
	InputComponent->BindAxis("Rotate", this, &AAP_Pawn::rotatePawn);

	// Set up "look" bindings.
	InputComponent->BindAxis("Turn", this, &AAP_Pawn::turn);
	InputComponent->BindAxis("LookUp", this, &AAP_Pawn::lookUp);
}

void AAP_Pawn::rotatePawn(float r) {
	angle = r;
}

void AAP_Pawn::moveX(float x) {
	isMovingFwd = x != 0.0f ? true : false;
//	UE_LOG(LogTemp, Warning, TEXT("ismoving = %d"), isMoving)
	currVel = GetActorForwardVector() * x * SPEED;
}

void AAP_Pawn::moveY(float y) {
	if (!isMovingFwd) 
		isMovingRgt = y != 0.0f ? true : false;
//	UE_LOG(LogTemp, Warning, TEXT("ismoving = %d"), isMoving)
	currVel += GetActorRightVector() * y * SPEED;
}

void AAP_Pawn::turn(float t) {
	FRotator newRot = GetActorRotation();
	newRot.Yaw += t;	//rotate about Z axis
	SetActorRotation(newRot);
}

void AAP_Pawn::lookUp(float l) {
	FRotator NewRotation = camera->GetComponentRotation();
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + l, -50.0f, 10.0f);	//limit pitch rotation
	camera->SetWorldRotation(NewRotation);
}

void AAP_Pawn::fire() {		// Attempt to fire a projectile.
	FVector projLoc = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	FRotator projRot = FRotator::ZeroRotator;

	UWorld* World = GetWorld();
	if (World) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;
		AProjectile *proj = World->SpawnActor<AProjectile>(ProjectileClass, projLoc, projRot, SpawnParams);

		if (proj) {	// Set the projectile's initial trajectory.
			float p = camera->GetRelativeTransform().Rotator().Pitch + 45.0f;
			FVector dir = GetActorForwardVector().RotateAngleAxis(p, GetActorRightVector());
			proj->FireInDirection(dir.GetSafeNormal());	
		}
	}
}

void AAP_Pawn::OnBeginOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult){
//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Pawn trigger overlap"));
	FString name = OtherActor->GetFName().ToString();
	if (name.Contains("Wall")) {	//only chk collisions with wall
		FVector dir = GetActorForwardVector(); dir.Z = 0.0f;
		FRotator curRot = GetActorRotation();
		FVector curPos = GetActorLocation();
		FVector endTrace = curPos + dir*rad;

		FCollisionQueryParams TraceParams(FName(TEXT("collCapTrace")), true);
		TraceParams.bTraceComplex = true;
		//			TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;
		//Ignore Actors
		//			TraceParams.AddIgnoredActor(actorToIgnore);

		FHitResult hit = FHitResult(ForceInit); //Re-initialize hit info
	//			DrawDebugCapsule(GetWorld(), curPos, rad / 3.0f, 10.0f, FQuat(), FColor::Cyan, true, 3.0f);		//**** CRASHES!!!
		GetWorld()->SweepSingleByChannel(hit, curPos, endTrace, FQuat(), ECC_Visibility, FCollisionShape::MakeCapsule(rad / 3.0f, 10.0f), TraceParams);
		UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *hit.ToString());

		SetActorLocation(curPos + hit.Normal * 10.0f);	//move away from wall!
		FVector newDir = dir - 2.0f*FVector::DotProduct(dir, hit.Normal) * hit.Normal;
//		SetActorRotation(newDir.Rotation());	//immediately rotate away from wall
		targRot = newDir.Rotation(); targRot.Pitch = targRot.Roll = 0.0f;
		interp = true;
	}
}

void AAP_Pawn::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex){
}
