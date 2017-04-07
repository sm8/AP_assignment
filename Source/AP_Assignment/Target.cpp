// SM

#include "AP_Assignment.h"
#include "Target.h"


// Sets default values
ATarget::ATarget(){
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	collisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	// Set the capsule's collision radius and half-height.
	collisionComponent->SetCapsuleSize(20.0f, 50.0f);
	RootComponent = collisionComponent;

//	collisionComponent->SetSimulatePhysics(true);	//****
	collisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	aiPercComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("aiPercComponent"));
	sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	sightConfig->SightRadius = 200.0f;
	sightConfig->LoseSightRadius = 220.0f;
	sightConfig->PeripheralVisionAngleDegrees = 90.0f;
	sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	sightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	sightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	aiPercComp->ConfigureSense(*sightConfig);
	aiPercComp->SetDominantSense(sightConfig->GetSenseImplementation());
	aiPercComp->OnPerceptionUpdated.AddDynamic(this, &ATarget::OnSensed);

	currVel = FVector::ZeroVector;
	speed = 130.0f;

	isMoving = false;
	interp = false;
	rotationSpeed = 1.0f;
}

// Called when the game starts or when spawned
void ATarget::BeginPlay(){
	Super::BeginPlay();

	FConstPawnIterator pIterator = GetWorld()->GetPawnIterator();
	APawn* pawn = *pIterator;	//as we know ONLY 1 pawn obj!
	FVector newD = pawn->GetActorLocation() - GetActorLocation();	//find diff 'tween pawn and Target loc
	newD.Z = 0.0f;	//we are NOT interested in any Z diff
	FRotator rotDir = newD.Rotation() - GetActorForwardVector().Rotation();	//calc diff in reqd dir & curr. dir
	rotDir.Normalize();	//ensure rotation is a unit vector
	SetActorRotation(rotDir);
	//draw a debug line to check it goes from Target to Pawn in 'new' dir
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector()*200.0f, FColor::Red, true, 5.0f, 0, 1.0f);
}

// Called every frame
void ATarget::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );

	// Handle movement based on where Target is to baseLoc
	if (!currVel.IsZero()) {
		FVector newLocation = GetActorLocation() + currVel * DeltaTime;
//		UE_LOG(LogTemp, Warning, TEXT("diff sq=%f dt=%f currVel: %s"), (newLocation - baseLoc).SizeSquared2D(), DeltaTime, *currVel.ToString());
		if ((newLocation - baseLoc).SizeSquared2D() > 1.0f) {	//set if not at base
			SetActorLocation(newLocation);
			isMoving = true;
			speed = 30.0f;
		} else {
			currVel = FVector::ZeroVector;	//****
			isMoving = false;
			speed = 130.0f;
		}
	}

	if (interp) {
		FRotator nr = FMath::RInterpTo(GetActorForwardVector().Rotation(), targRot, DeltaTime, rotationSpeed);
		if ((nr - targRot).IsNearlyZero()) {
			nr = targRot;
			interp = false;
		}
		SetActorRotation(nr);
	}
}

void ATarget::OnSensed(TArray<AActor*> testActors) {
	for (int i = 0; i < testActors.Num(); i++) {
		FString n = testActors[i]->GetName();
		FActorPerceptionBlueprintInfo info;
		aiPercComp->GetActorsPerception(testActors[i], info);
		interp = true;
		if (info.LastSensedStimuli[0].WasSuccessfullySensed()) {
			FVector dir = testActors[i]->GetActorLocation() - GetActorLocation();
			dir.Z = 0.0f;	//reset as they may be at diff Z values
			currVel = dir.GetSafeNormal() * speed;	//set vel towards actor
			setNewRotation(this, testActors[i]->GetActorLocation(), GetActorLocation()); //rotate
		} else {	//actor has moved away from radius
			speed = 30.0f;
			FVector dir = baseLoc - GetActorLocation();
			dir.Z = 0.0f;	//reset as they may be at diff Z values
			if (dir.SizeSquared2D() > 1.0f) {	//set vel towards baseLoc
				currVel = dir.GetSafeNormal() * speed;
			} else
				currVel = FVector::ZeroVector;
			setNewRotation(this, baseLoc, GetActorLocation());	//rot
		}
	}
}


void ATarget::setNewRotation(AActor *actorToRotate, FVector targPos, FVector currPos) {
	FVector newD = targPos - currPos;	//diff in curr loc and Target loc
	newD.Z = 0.0f;	//we are NOT interested in any Z diff
	FRotator rotDir = newD.Rotation() - actorToRotate->GetActorForwardVector().Rotation(); //calc diff in reqd dir & curr. dir
	targRot = newD.Rotation();
	if (interp)
		targRot = newD.Rotation();
	else
		actorToRotate->SetActorRotation(newD.Rotation()); //rot with NO interp
}




