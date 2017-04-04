// SM

#include "AP_Assignment.h"
#include "Wall.h"
#include "AP_Pawn.h"	//****

// Sets default values
AWall::AWall(){
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	collisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("collisionComponent"));
	RootComponent = collisionComponent;
	collisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));
	collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AWall::OnBeginOverlap);
	collisionComponent->OnComponentEndOverlap.AddDynamic(this, &AWall::OnOverlapEnd);

	staticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("staticMeshComponent"));
	rad = 100.0f;
}

// Called when the game starts or when spawned
void AWall::BeginPlay(){
	Super::BeginPlay();
	
}

// Called every frame
void AWall::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );

}

void AWall::OnBeginOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) {
//	UE_LOG(LogTemp, Warning, TEXT("SweepResult: %s"), *SweepResult.ToString());
//
//	if (OtherActor != this) {
//		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Trigger overlap"));
//		AAP_Pawn *p = (AAP_Pawn*)OtherActor;
//		if (p) {
//			FVector dir = OtherActor->GetActorForwardVector(); dir.Z = 0.0f;
//			FRotator curRot = OtherActor->GetActorRotation();
//			FVector curPos = OtherActor->GetActorLocation();
//			FVector endTrace = curPos + dir*rad;
//
//			FCollisionQueryParams TraceParams(FName(TEXT("collSphereTrace")), true);
//			TraceParams.bTraceComplex = true;
////			TraceParams.bTraceAsyncScene = true;
//			TraceParams.bReturnPhysicalMaterial = false;
//			//Ignore Actors
////			TraceParams.AddIgnoredActor(actorToIgnore);
//
//			FHitResult hit = FHitResult(ForceInit); //Re-initialize hit info
////			FCollisionShape cap = FCollisionShape::MakeCapsule(rad / 3.0f, 10.0f);
////			DrawDebugCapsule(GetWorld(), curPos, cap.GetCapsuleHalfHeight(), cap.GetCapsuleRadius(), FQuat(), FColor::Cyan, true, 3.0f);
////			DrawDebugCapsule(GetWorld(), curPos, rad / 3.0f, 10.0f, FQuat(), FColor::Cyan, true, 3.0f);		//**** CRASHES!!!
//
////			GetWorld()->SweepSingleByChannel(hit, curPos, endTrace, FQuat(), ECC_Visibility, FCollisionShape::MakeSphere(rad), TraceParams); //gets floor collision!
//			GetWorld()->SweepSingleByChannel(hit, curPos, endTrace, FQuat(), ECC_Visibility, FCollisionShape::MakeCapsule(rad/3.0f, 10.0f), TraceParams); 
//			UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *hit.ToString());
//
//			FVector box = this->collisionComponent->GetScaledBoxExtent();
//			UE_LOG(LogTemp, Warning, TEXT("box: %s"), *box.ToString());
//
//			OtherActor->SetActorLocation(curPos + hit.Normal * 10.0f);	//move away from wall!
//			//float rotAng = FVector::DotProduct(curRot.Vector(), hit.Normal);
//			//FVector rotAxis = FVector::CrossProduct(curRot.Vector(), hit.Normal); rotAxis.X = rotAxis.Y = 0.0f;
//			//FVector newDir = curRot.Vector().RotateAngleAxis(rotAng, rotAxis);
//			FVector newDir = dir - 2.0f*FVector::DotProduct(dir, hit.Normal) * hit.Normal;
//			OtherActor->SetActorRotation(newDir.Rotation());	//rotate away from wall
//		}
//	}
}

void AWall::OnOverlapEnd(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex){
}


