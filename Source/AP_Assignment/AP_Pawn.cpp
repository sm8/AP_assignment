// SM

#include "AP_Assignment.h"
#include "AP_Pawn.h"
#include "Target.h"
#include "Perception/AISense_Sight.h"	//used to register this pawn
#include "Wall.h"	//****
#include "FileHelpers.h"	//for file I/O
//#include "HighResScreenShot.h"
//
////
//// Taken from: https://forums.unrealengine.com/showthread.php?104816-How-to-save-UTexture2D-to-PNG-file
////
//void SaveTexture2DDebug(const uint8* PPixelData, int width, int height, FString Filename) {
//	TArray<FColor> OutBMP;
//	int w = width;
//	int h = height;
//
//	OutBMP.InsertZeroed(0, w*h);
//
//	for (int i = 0; i < (w*h); ++i) {
//		uint8 R = PPixelData[i * 4 + 2];
//		uint8 G = PPixelData[i * 4 + 1];
//		uint8 B = PPixelData[i * 4 + 0];
//		uint8 A = PPixelData[i * 4 + 3];
//
//		OutBMP[i].R = R;
//		OutBMP[i].G = G;
//		OutBMP[i].B = B;
//		OutBMP[i].A = A;
//	}
//
//	FIntPoint DestSize(w, h);
//
//	FString ResultPath;
//	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
//	bool bSaved = HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);
//
//	UE_LOG(LogTemp, Warning, TEXT("SaveTexture2DDebug: %d %d"), w, h);
//	UE_LOG(LogTemp, Warning, TEXT("SaveTexture2DDebug: %s %d"), *ResultPath, bSaved == true ? 1 : 0);
//}
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

	isMovingFwd = isMovingRgt = interp = false;
	rad = 100.0f;
	rotationSpeed = 1.0f;
//	totTime = prevTime = maxTimeAtPos = 0.0f;
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

//	//**** for Heatmap
//	FVector orig, maxB;
//	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
////		UE_LOG(LogTemp, Warning, TEXT("name:  %s"), *ActorItr->GetName());
//		if (ActorItr->GetName().Contains("Floor")) {	//get sizes for Heatmap
//			ActorItr->GetActorBounds(true, orig, maxB);
//			maxX = maxB.X; maxY = maxB.Y;
//			minX = -maxX; minY = -maxY;
//		}
//		else {	//find ALL STATIC mesh comps, so can chk whether static mobility
//			TArray<UStaticMeshComponent*> Components;
//			ActorItr->GetComponents<UStaticMeshComponent>(Components);
//			for (int32 i = 0; i<Components.Num(); i++){
//				UStaticMeshComponent* StaticMeshComponent = Components[i];
//				if (!StaticMeshComponent->Mobility) { //assume static is 0
//					ActorItr->GetActorBounds(true, orig, maxB);	//chk bounds of static obj
//					if(maxB.X > 0.0f && maxB.Y > 0.0f)	//Eg sky sphere is static but has 0 bounds
//						staticActors.Add(new ActorAndBounds(*ActorItr, orig, maxB));
//				}
//			}
//		}
//	}
//	GetActorBounds(true, orig, maxB);
//	pawnRad = maxB.X;
//	h = w = 1024;	//****NOMINAL
}

// Called every frame
void AAP_Pawn::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );

	totTime += DeltaTime;	//used for Heatmap

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
//		updatePositionData(newLocation);	//for Heatmap
	}
	// Handle angular movement based on Rotate
	if (angle != 0.0f) {
		FRotator newRot = GetActorRotation();
		newRot.Yaw += angle;	//rotate about Z axis
		SetActorRotation(newRot);
	}
}

void AAP_Pawn::updatePositionData(FVector &newLocation){
	float diffT = totTime - prevTime;	//****for Heatmap data
	if (diffT > maxTimeAtPos) maxTimeAtPos = diffT;
	PosData p(newLocation.X, newLocation.Y, newLocation.Z, diffT);
	pawnPs.Add(p);	//add to dyn array
	FString d = FString::Printf(TEXT("%f,%f,%f,%f"), newLocation.X, newLocation.Y, newLocation.Z, diffT);
	pawnPositions.Add(d);	//add to dyn string array
	allPawnPos += d + "\r\n";	//append for txt output
	prevTime = totTime;
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
//	angle = r;
//	if (r != 0.0f) {
//		updateLastPositionInArrays();
//
//		uint8 *pixels = new uint8[w*h*4];	//4*8bits for each colour & alpha
//		for (int i = 0; i < w*h*4; i++) pixels[i] = 0;	//init
//
//		float gx = (maxX - minX) / (float)w;  	//calc 'grid' sizes
//		float gy = (maxY-minY) / (float)h;
//
//		const int NUM_CHKS = w / 7;	//num of times to calc ang around ctr
//		float angle = FMath::DegreesToRadians(360.0f / ((float)NUM_CHKS));
//		float tr, rx, ry;	//****
//		for (int i = 0; i < pawnPs.Num(); i++) {	//process each pawn pos
//			tr = 0.0f;	//start at ctr. This is inefficient!
//			while (tr < pawnRad) {	
//				for (int k = 0; k < NUM_CHKS; k++) {	//chk around ctr
//					rx = pawnPs[i].x; ry = pawnPs[i].y;
//					rx += tr*FMath::Cos((float)k*angle);
//					ry += tr*FMath::Sin((float)k*angle);
//					int xp = getGridPos(rx, minX, gx);	//calc array pos
//					int yp = getGridPos(ry, minY, gy);
//					uint8 newColour = (uint8)(254.0f * pawnPs[i].dt / maxTimeAtPos) + 1;
//					int oldColour = pixels[4 * (xp + yp*w) + 2];
//					if (newColour > oldColour) pixels[4 * (xp + yp*w) + 2] = newColour;	//Red
////					pixels[4 * (xp + yp*w) + 2] |= newColour;	//Red
//					pixels[4 * (xp + yp*w)] = 10;	//set blue
//					pixels[4 * (xp + yp*w) + 1] = 100;	//set green
//					pixels[4 * (xp + yp*w) + 3] = 255;	//set alpha
//				}
//				tr += gx*0.5f;	//increase rad by half grid width each time
//			}
//		}
//		FString p = FPlatformMisc::GameDir();	//get base folder of project
//		addStaticBoundsToHeatmap(pixels, gx, gy);	//add static objs to Heatmap
//		FFileHelper::SaveStringToFile(allPawnPos, *FString::Printf(TEXT("%sHeatmapPos.txt"),*p));	//save x,y,z & dt
////		outputArrayCSVfile(w, h, pixels, p + "Heatmap.csv");	//for testing / debugging
//		SaveTexture2DDebug(pixels, w, h, p + "Heatmap.png");	//create Heatmap as PNG
//	}
}
////
//// Draw STATIC objs on Heatmap, in grey - assume AABBs. 
//// 
//void AAP_Pawn::addStaticBoundsToHeatmap(uint8 *pixels, float gx, float gy){
//	float tx, ty, rx, ry;
//	for (int32 i = 0; i < staticActors.Num(); i++) {	//****test array
//		UE_LOG(LogTemp, Warning, TEXT("Static actor name:  %s maxB: %s orig: %s"), *staticActors[i]->actor->GetName(), *staticActors[i]->maxBounds.ToString(), *staticActors[i]->org.ToString());
//		tx = ty = 0.0f;
//		while (tx <= staticActors[i]->maxBounds.X){
//			rx = staticActors[i]->org.X - staticActors[i]->maxBounds.X + 2.0f*tx;
//			ty = 0.0f;
//			while (ty <= staticActors[i]->maxBounds.Y) {
//				ry = staticActors[i]->org.Y - staticActors[i]->maxBounds.Y + 2.0f*ty;
//				int xp = getGridPos(rx, minX, gx);	//calc array pos
//				int yp = getGridPos(ry, minY, gy);
//				pixels[4 * (xp + yp*w) + 2] = 50;	//draw STATIC obj in grey
//				pixels[4 * (xp + yp*w)] = 50;	
//				pixels[4 * (xp + yp*w) + 1] = 50;	
//				pixels[4 * (xp + yp*w) + 3] = 100;	
//				ty += gy*0.5f;	//increase rad by half grid width each time
//			}
//			tx += gx*0.5f;
//		}
//	}
//}

//// needed, as last pos not set in file because NO pos chg.
//void AAP_Pawn::updateLastPositionInArrays(){
//	float diffT = totTime - prevTime;	//get LAST pos
//	if (diffT > maxTimeAtPos) maxTimeAtPos = diffT;
//	pawnPs[pawnPs.Num() - 1].dt = diffT;	//change last array time
//	FVector newLocation = GetActorLocation();
//	FString d = FString::Printf(TEXT("%f,%f,%f,%f"), newLocation.X, newLocation.Y, newLocation.Z, diffT);
//	pawnPositions.RemoveAt(pawnPositions.Num() - 1);	//remove last pos
//	pawnPositions.Add(d);	//add to dyn string array
//	allPawnPos += d + "\r\n";	//append for txt output
//}
//
//void AAP_Pawn::outputArrayCSVfile(int w, int h, uint8 *pixels, FString filename){
//	FString arrayOut = "Num,Value\n";
//	for (int i = 0; i < w*h * 4; i++)
//		arrayOut += FString::Printf(TEXT("%d,%d\n"), i, pixels[i]);
//	arrayOut += FString::Printf(TEXT("\nMin,=MIN(B2:B%d)"), w*h * 4 + 1);	//For Excel, output Summary stats
//	arrayOut += FString::Printf(TEXT("\nMax,=MAX(B2:B%d)"), w*h * 4 + 1);
//	arrayOut += FString::Printf(TEXT("\nAvg,=AVERAGE(B2:B%d)\n"), w*h * 4 + 1);
//	FFileHelper::SaveStringToFile(arrayOut, *filename);
//}
//
//unsigned int AAP_Pawn::getGridPos(float rx, float minX, float gx){
//	float epsilon = 0.001f;	//for possible error in calcs
//	float px = (rx - minX) / (gx + epsilon);	//calc grid pos
//	return (unsigned int)px + 1;
//}

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
//		DrawDebugCapsule(GetWorld(), curPos, rad / 3.0f, 10.0f, FQuat(), FColor::Cyan, true, 3.0f);		//**** CRASHES!!!
//		DrawDebugCapsule(GetWorld(), curPos, rad / 3.0f, 10.0f, FQuat::Identity, FColor::Cyan, true, 3.0f);		
		GetWorld()->DebugDrawTraceTag = TEXT("collCapTrace");	//shows collision with shape
		GetWorld()->SweepSingleByChannel(hit, curPos, endTrace, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(rad / 3.0f, 10.0f), TraceParams);
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
