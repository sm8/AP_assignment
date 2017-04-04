// SM

#include "AP_Assignment.h"
#include "TriggerCube.h"
#include "EngineUtils.h"	//****for ActorIterator
#include "Runtime/Engine/Classes/PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
ATriggerCube::ATriggerCube(){
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	collisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("collisionComponent"));
	RootComponent = collisionComponent;
	collisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ATriggerCube::OnBeginOverlap);
	collisionComponent->OnComponentEndOverlap.AddDynamic(this, &ATriggerCube::OnOverlapEnd);

	//axleBoardConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("axleBoardConstraint"));
	//axleBoardConstraint->AttachTo(RootComponent);
	constraint1 = constraint2 = nullptr;
	rotSpeed = 20.0f;
	angImpulse = FVector(50000000.0f, 0.0f, 0.0f);
}

// Called when the game starts or when spawned
void ATriggerCube::BeginPlay(){
	Super::BeginPlay();

	//TArray<UActorComponent*> StaticMeshComponents = this->GetComponentsByClass(UStaticMeshComponent::StaticClass());

	//UPhysicsConstraintComponent* CurrentConstraint = nullptr;
	//UStaticMeshComponent* CurrentStaticMeshComponent = nullptr;
	//CurrentConstraint = axleBoardConstraint;
	//if (CurrentConstraint){
	//	if (CurrentConstraint->ComponentTags.Num() > 0)	{
	//		for (int32 j = 0; j < StaticMeshComponents.Num(); j++){
	//			CurrentStaticMeshComponent = (UStaticMeshComponent*)StaticMeshComponents[j];
	//			if (CurrentStaticMeshComponent){
	//				if (CurrentStaticMeshComponent->ComponentTags.Num() > 0){
	//					if (CurrentConstraint->ComponentTags[0].ToString().Equals(CurrentStaticMeshComponent->ComponentTags[0].ToString(), ESearchCase::IgnoreCase)){
	//						staticMeshComponent = CurrentStaticMeshComponent;
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(parent);

	//FConstraintInstance constraintInstance;

	//constraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, length);
	//constraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, length);
	//constraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, length);

	//constraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
	//constraintInstance.ProfileInstance.LinearLimit.Stiffness = 1000.f;
	//constraintInstance.ProfileInstance.LinearLimit.Damping = .5f;

	//constraint->ConstraintInstance = constraintInstance;

	TArray<UMaterialInstance*>* result = new TArray<UMaterialInstance*>();
	
	UObjectLibrary *lib = UObjectLibrary::CreateLibrary(UMaterialInstance::StaticClass(), false, true);
	UE_LOG(LogTemp, Warning, TEXT("Searching for material instances in /Game/Materials..."));
	lib->LoadAssetDataFromPath(TEXT("/Game/"));
	TArray<FAssetData> assetData;
	lib->GetAssetDataList(assetData);
	UE_LOG(LogTemp, Warning, TEXT("Found %d"), assetData.Num());
	
	for (FAssetData asset : assetData) {
		UMaterialInstance* mi = Cast<UMaterialInstance>(asset.GetAsset());
		if (mi) {
			UE_LOG(LogTemp, Warning, TEXT("Material instance %s"), *mi->GetName());
			result->Add(mi);
		}
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 20, FColor::Green, true, -1.0f);
}

// Called every frame
void ATriggerCube::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );
	bool interp = true;	//****

//	if (interp) {
//		if (constraint2) {	//only if available
//			FRotator nr = FMath::RInterpTo(constraint2->GetActorRotation(), targRot, DeltaTime, rotSpeed);
//			if ((nr - targRot).IsNearlyZero()) {
//				nr = targRot;
//				interp = false;
//			}
//			//FTransform newT = constraint2->GetActorTransform();
//			//FQuat q = (FQuat)nr;
//			//q.Normalize();
//
//			//FVector axLoc = constraint1->GetActorLocation();
//
//			//FVector oldLoc = constraint2->GetActorLocation();
//			//newT.AddToTranslation(-constraint1->GetActorLocation());
//			//newT.SetRotation(q);
//			//constraint2->SetActorTransform(newT);
//
//			//oldLoc = constraint2->GetActorLocation();
//			//nr = constraint2->GetActorRotation();
//
//			//newT.AddToTranslation(constraint1->GetActorLocation());
//			//constraint2->SetActorTransform(newT);
//
//
////			FMatrix finalMat;
////			FTranslationMatrix tm = FTranslationMatrix(-constraint1->GetActorLocation());
////			FRotationMatrix rm = FRotationMatrix(nr);
////			finalMat = tm * rm;
//////			constraint2->GetStaticMeshComponent()->SetWorldLocationAndRotation(finalMat.GetOrigin(), finalMat.Rotator());
////
////			//tm = FTranslationMatrix(constraint1->GetActorLocation());
////			//rm = FRotationMatrix(FRotator::ZeroRotator);
////			finalMat = tm * rm * FTranslationMatrix(constraint1->GetActorLocation());
//////			constraint2->GetStaticMeshComponent()->SetWorldLocationAndRotation(pos, finalMat.Rotator());
////			UE_LOG(LogTemp, Warning, TEXT("mat: %s"), *finalMat.ToString());
//
//			FVector oldLoc = constraint2->GetActorLocation();
//			FVector cDiff = constraint2->GetActorLocation() - constraint1->GetActorLocation();
//			constraint2->SetActorLocation(-constraint1->GetActorLocation());
//			constraint2->SetActorRotation(nr);
//			constraint2->SetActorLocation(constraint1->GetActorLocation());
//			FVector newLoc = constraint2->GetActorLocation();
//
//			constraint2->SetActorLocation(constraint2->GetActorLocation() + cDiff);
//		}
//	}
}

void ATriggerCube::setNewRotation(AStaticMeshActor *actorToRotate, float rotAng, bool interp) {
	FRotator rotDir(rotAng,0.0f,0.0f); //calc diff in reqd dir & curr. dir
	if (interp)
		targRot = rotDir;
	else
		actorToRotate->SetActorRotation(rotDir); //rot with NO interp
}

void ATriggerCube::OnBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult){
	if (OtherActor != this) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Trigger overlap"));
		for (TActorIterator<AStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
			// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
			UE_LOG(LogTemp, Warning, TEXT("name:  %s"), *ActorItr->GetName());
			if (ActorItr->GetName() == "Axle")
				constraint1 = *ActorItr;
			if (ActorItr->GetName() == "Launchboard")
				constraint2 = *ActorItr;
		}
		rotPt = constraint1->GetActorLocation();
//		targRot = constraint2->GetActorRotation();

		constraint1->GetStaticMeshComponent()->AddAngularImpulse(angImpulse);
//		setNewRotation(constraint2, 90.0f, true);

//		constraint2->SetActorRotation(FRotator(90.0f, 0.0f, 0.0f));
//		constraint2->SetActorLocation(FVector(90.0f, 0.0f, 0.0f));
	}
}

void ATriggerCube::CreateNewPhysicsConstraintBetween(AStaticMeshActor* RootSMA, AStaticMeshActor* TargetSMA){
	//set up the constraint instance with all the desired values
	FConstraintInstance ConstraintInstance;

	//set values here, see functions I am sharing with you below
	//UYourStaticLibrary::SetLinearLimits(ConstraintInstance, ...); //or make the functions below non static
	//UYourStaticLibrary::SetAngularLimits(ConstraintInstance, ...);

	//New Object
	UPhysicsConstraintComponent* ConstraintComp = NewObject<UPhysicsConstraintComponent>(RootSMA);
	if (!ConstraintComp){
		//UE_LOG constraint UObject could not be created!
		return;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~
	//Set Constraint Instance!
	ConstraintComp->ConstraintInstance = ConstraintInstance;
	//~~~~~~~~~~~~~~~~~~~~~~~~

	//Set World Location
	ConstraintComp->SetWorldLocation(RootSMA->GetActorLocation());

	//Attach to Root!
	ConstraintComp->AttachTo(RootSMA->GetRootComponent(), NAME_None, EAttachLocation::KeepWorldPosition);

	//~~~ Init Constraint ~~~
	ConstraintComp->SetConstrainedComponents(RootSMA->GetStaticMeshComponent(), NAME_None, TargetSMA->GetStaticMeshComponent(), NAME_None);
}

void ATriggerCube::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex){
	if (OtherActor != this) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Trigger END overlap"));
	}
}



