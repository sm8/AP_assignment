// SM

#include "AP_Assignment.h"
#include "AP_Pawn.h"
#include "Target.h"
#include "Perception/AISense_Sight.h"	//used to register this pawn
#include "Wall.h"	//****
#include "FileHelpers.h"
#include "HighResScreenShot.h"

void SaveTexture2DDebug(const uint8* PPixelData, int width, int height, FString Filename) {
	TArray<FColor> OutBMP;
	int w = width;
	int h = height;

	OutBMP.InsertZeroed(0, w*h);

	for (int i = 0; i < (w*h); ++i) {
		uint8 R = PPixelData[i * 4 + 2];
		uint8 G = PPixelData[i * 4 + 1];
		uint8 B = PPixelData[i * 4 + 0];
		uint8 A = PPixelData[i * 4 + 3];

		OutBMP[i].R = R;
		OutBMP[i].G = G;
		OutBMP[i].B = B;
		OutBMP[i].A = A;
	}

	FIntPoint DestSize(w, h);

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	bool bSaved = HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);

	UE_LOG(LogTemp, Warning, TEXT("SaveTexture2DDebug: %d %d"), w, h);
	UE_LOG(LogTemp, Warning, TEXT("SaveTexture2DDebug: %s %d"), *ResultPath, bSaved == true ? 1 : 0);
}
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
	totTime = prevTime = 0.0f;
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

	totTime += DeltaTime;	//****

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
		PosData p(newLocation.X, newLocation.Y, newLocation.Z, totTime - prevTime);
		pawnPs.Add(p);
		FString d = FString::Printf(TEXT("%f,%f,%f,%f"), newLocation.X, newLocation.Y, newLocation.Z, totTime-prevTime);
		pawnPositions.Add(d);
		allPawnPos += d + "\r\n";
		prevTime = totTime;
		//IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		//FFileHelper::SaveStringToFile(d, TEXT("pawnPos.txt"),FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get());

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
//	angle = r;
	if (r != 0.0f) {
		FFileHelper::SaveStringToFile(allPawnPos, TEXT("pawnPos.txt"));	//save x,y,z,dt vals to txt file
		int w, h; 
		h = w = 256;	//****NOMINAL
		uint8 *pixels = new uint8[w*h*4];	//4*8bits for each colour & alpha
		float gx, gy, *fPpixels = new float[w*h];
//		for (int i = 0; i < w*h*4; i++) pixels[i] = 0;	//init

		float maxX, maxY; maxX = maxY = 1000.0f;	//assume floor +/- 1000
		gx = 2.0f*maxX / (float)w;  gy = 2.0f*maxY / (float)h;
		float minX, minY; minX = minY = -1000.0f;	//assume floor +/- 1000
		float epsilon = 0.001f;	//for error in calcs

		float lx, ly, hx, hy;
		lx = hx = pawnPs[0].x; ly = hy = pawnPs[0].y;
		for (int i = 1; i < pawnPs.Num(); i++) {
			if (pawnPs[i].x < lx) lx = pawnPs[i].x;	//get low / high vals in x & y
			if (pawnPs[i].x > hx) hx = pawnPs[i].x;
			if (pawnPs[i].y < ly) ly = pawnPs[i].y;
			if (pawnPs[i].y > hy) hy = pawnPs[i].y;
	
			float tr = gx*0.5f, rad = 50.0f, rx, ry;	//****
//			const int NUM_CHKS = 8;
//			float angle = FMath::DegreesToRadians(360.0f / ((float)NUM_TARGETS));
			while (tr < rad) {
//FVector targLoc = GetActorLocation() + FVector(500.0f*FMath::Cos((float)i*angle), 500.0f*FMath::Sin((float)i*angle), 50.0f);
				for (int k = 0; k < 5; k++) {	//chk middle & each corner about ctr too
					rx = pawnPs[i].x; ry = pawnPs[i].y;
					if (k == 1) { rx += tr; ry += tr; }	//chk each corner
					if (k == 2) { rx -= tr; ry += tr; }
					if (k == 3) { rx -= tr; ry -= tr; }
					if (k == 4) { rx += tr; ry -= tr; }
					float px = (rx - minX) / (gx + epsilon);	//calc grid pos
					int xp = (int)px + 1;
					float py = (ry - minY) / (gy + epsilon);
					int yp = (int)py + 1;
					for (int j = 0; j < 3; j++)
						pixels[4 * (xp + yp*w) + j] = 80;	//add 1 to grid counts
					pixels[4 * (xp + yp*w) + 3] = 255;	//set alpha
				}
				tr += gx*0.5f;
			}
		}
		FFileHelper::SaveStringToFile(allPawnPos, TEXT("pawnPos.txt"));
		FString arrayOut = "Num,Value\n";

		for (int i = 0; i < w*h * 4; i++)
			arrayOut += FString::Printf(TEXT("%d,%d\n"), i, pixels[i]);
		arrayOut += FString::Printf(TEXT("\nMin,=MIN(B2:B%d)\n"), w*h * 4 + 1);	//For Excel, output Summary stats
		arrayOut += FString::Printf(TEXT("\nMax,=MAX(B2:B%d)\n"), w*h * 4 + 1);
		arrayOut += FString::Printf(TEXT("\nAvg,=AVERAGE(B2:B%d)\n"), w*h * 4 + 1);
		FFileHelper::SaveStringToFile(arrayOut, TEXT("pawnPos.csv"));

		SaveTexture2DDebug(pixels, w, h, "newPawnPos.png");
	}
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
		FQuat q; q = FQuat::Identity;
//		DrawDebugCapsule(GetWorld(), curPos, rad / 3.0f, 10.0f, FQuat(), FColor::Cyan, true, 3.0f);		//**** CRASHES!!!
//		DrawDebugCapsule(GetWorld(), curPos, rad / 3.0f, 10.0f, FQuat::Identity, FColor::Cyan, true, 3.0f);		
		GetWorld()->DebugDrawTraceTag = TEXT("collCapTrace");	//shows collision with shape
//		GetWorld()->SweepSingleByChannel(hit, curPos, endTrace, FQuat(), ECC_Visibility, FCollisionShape::MakeCapsule(rad / 3.0f, 10.0f), TraceParams);
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
