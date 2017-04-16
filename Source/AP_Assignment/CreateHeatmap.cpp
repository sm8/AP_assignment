// SM

#include "AP_Assignment.h"
#include "CreateHeatmap.h"
#include "FileHelpers.h"	//for file I/O
#include "HighResScreenShot.h"

//
// Taken from: https://forums.unrealengine.com/showthread.php?104816-How-to-save-UTexture2D-to-PNG-file
//
void ACreateHeatmap::SaveTexture2DDebug(const uint8* PPixelData, int width, int height, FString Filename) {
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
ACreateHeatmap::ACreateHeatmap(){
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	totTime = prevTime = maxTimeAtPos = playerRad = maxX = 0.0f;
	nameOfPlatform = "Floor";
	nameOfPlayerToTrack = "Player";
	player = NULL;
	platform = NULL;
	prevLoc = FVector::ZeroVector;
	heatMapProcessed = false;
	widthOfHeatmapInPixels = heightOfHeatmapInPixels = 1024;
	playerMoveCol = FColor(0, 100, 10, 255);	//default colours for player movement
	staticObjCol = FColor(50, 50, 50, 100);	//by default, draw STATIC obj in grey
}

// Called when the game starts or when spawned
void ACreateHeatmap::BeginPlay(){
	Super::BeginPlay();
	
	//**** for Heatmap
	FVector orig, maxB, highMax = FVector::ZeroVector; 
	w = widthOfHeatmapInPixels; h = heightOfHeatmapInPixels;
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
		//		UE_LOG(LogTemp, Warning, TEXT("name:  %s"), *ActorItr->GetName());
		if (ActorItr->GetName().Contains(nameOfPlatform)) {	//get sizes for Heatmap
			platform = *ActorItr;
			ActorItr->GetActorBounds(true, orig, maxB);
			maxX = maxB.X; maxY = maxB.Y;
			minX = -maxX; minY = -maxY;
		} else if (ActorItr->GetName().Contains(nameOfPlayerToTrack)) {
			player = (APawn*)*ActorItr;
			player->GetActorBounds(true, orig, maxB);	//CHANGE to player controller???
			playerRad = maxB.X;
		}
		else {	//find ALL STATIC mesh comps, so can chk whether static mobility
			TArray<UStaticMeshComponent*> Components;
			ActorItr->GetComponents<UStaticMeshComponent>(Components);
			for (int32 i = 0; i<Components.Num(); i++) {
				UStaticMeshComponent* StaticMeshComponent = Components[i];
				if (!StaticMeshComponent->Mobility) { //assume static is 0
					ActorItr->GetActorBounds(true, orig, maxB);	//chk bounds of static obj
					if (maxB.X*maxB.Y > highMax.X*highMax.Y) {
						platform = *ActorItr;
						highMax = maxB; //in case platform not found
					}
					if (maxB.X > 0.0f && maxB.Y > 0.0f)	//Eg sky sphere is static but has 0 bounds
						staticActors.Add(new ActorAndBounds(*ActorItr, orig, maxB));
				}
			}
		}
	}
	if (maxX == 0.0f) {	//if platform NOT found then use largest bounds found
		maxX = highMax.X; maxY = highMax.Y;
		minX = -maxX; minY = -maxY;
		for (int i = 0; i < staticActors.Num(); i++) {
			ActorAndBounds *ab = staticActors[i];
			if (ab->actor->GetName() == platform->GetName())
				staticActors.RemoveAt(i);
		}
	}
	if (player == NULL) {
		player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		player->GetActorBounds(true, orig, maxB);
		playerRad = maxB.X;
	}
	if (checkBindingIsSetup("SaveHeatmap"))
		player->InputComponent->BindAxis(TEXT("SaveHeatmap"));
	else {
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, "Heatmap cannot be created. Axis mapping: SaveHeatmap has NOT been setup. Use Project settings, Input to set it up");
		return;
	}
}

bool ACreateHeatmap::checkBindingIsSetup(FString axisMapName){
	const UInputSettings* InputSettings = GetDefault<UInputSettings>();
	TArray<FName> axisMappingNames;
	InputSettings->GetAxisNames(axisMappingNames);
	for (int i = 0; i < axisMappingNames.Num(); i++) 
		if (axisMappingNames[i].ToString() == axisMapName)
			return true;
	return false;
}

// Called every frame
void ACreateHeatmap::Tick( float DeltaTime ){
	Super::Tick( DeltaTime );

	totTime += DeltaTime;	//used for Heatmap

	FVector newLocation = player->GetActorLocation();	//get PLAYER POS!!!!
	if ((newLocation - prevLoc).SizeSquared2D() > 0.01f) {
		updatePositionData(newLocation);	//for Heatmap
		prevLoc = newLocation;
	}

	float val = player->InputComponent->GetAxisValue(TEXT("SaveHeatmap"));
	if (val != 0.0f)
		saveHeatmap();
}

void ACreateHeatmap::saveHeatmap(){
	if (!heatMapProcessed) {
		updateLastPositionInArrays();

		uint8 *pixels = new uint8[w*h * BPP];	//4*8bits for each colour & alpha
		for (unsigned int i = 0; i < w*h * BPP; i++) pixels[i] = 0;	//init

		float gx = (maxX - minX) / (float)w;  	//calc 'grid' sizes
		float gy = (maxY - minY) / (float)h;
		addStaticBoundsToHeatmap(pixels, gx, gy);	//add static objs to Heatmap

		const int NUM_CHKS = w / 7;	//num of times to calc ang around ctr
		float angle = FMath::DegreesToRadians(360.0f / ((float)NUM_CHKS));
		float tr, rx, ry;
		for (int i = 0; i < playerPos.Num(); i++) {	//process each player pos
			tr = 0.0f;	//start at ctr. This is inefficient!
			while (tr < playerRad) {
				for (int k = 0; k < NUM_CHKS; k++) {	//chk around ctr
					rx = playerPos[i].x; ry = playerPos[i].y;
					rx += tr*FMath::Cos((float)k*angle);
					ry += tr*FMath::Sin((float)k*angle);
					int xp = getGridPos(rx, minX, gx);	//calc array pos
					int yp = getGridPos(ry, minY, gy);
					uint8 newColour = (uint8)(254.0f * playerPos[i].dt / maxTimeAtPos) + 1;
					int oldColour = pixels[BPP * (xp + yp*w) + 2];
					if (oldColour > newColour) newColour = oldColour;	//Red
					set32BitPixel(pixels, BPP * (xp + yp*w), newColour, playerMoveCol.G, playerMoveCol.B, playerMoveCol.A);
				}
				tr += gx*0.5f;	//increase rad by half grid width each time
			}
		}
		FString p = FPlatformMisc::GameDir();	//get base folder of project
		FFileHelper::SaveStringToFile(allPlayerPos, *FString::Printf(TEXT("%sHeatmapPos.txt"), *p)); //save pos & dt
	//	outputArrayCSVfile(w, h, pixels, p + "Heatmap.csv");	//for testing / debugging
		SaveTexture2DDebug(pixels, w, h, p + "Heatmap.png");	//create Heatmap as PNG
		heatMapProcessed = true;
		clearArrays(pixels);
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, *FString::Printf(TEXT("Heatmap created in: %sHeatmap.png"), *p));
	}
}

void ACreateHeatmap::clearArrays(uint8 * pixels){
	delete pixels;	//clear dyn memory
	allPlayerPos.Empty();
	playerPos.Empty();
	playerPositions.Empty();
}

//
// Draw STATIC objs on Heatmap, in grey - assume AABBs. 
// 
void ACreateHeatmap::addStaticBoundsToHeatmap(uint8 * pixels, float gx, float gy){
	float tx, ty, rx, ry;
	for (int32 i = 0; i < staticActors.Num(); i++) {	//****test array
		UE_LOG(LogTemp, Warning, TEXT("Static actor name:  %s maxB: %s orig: %s"), *staticActors[i]->actor->GetName(), *staticActors[i]->maxBounds.ToString(), *staticActors[i]->org.ToString());
		tx = ty = 0.0f;
		while (tx <= staticActors[i]->maxBounds.X) {
			rx = staticActors[i]->org.X - staticActors[i]->maxBounds.X + 2.0f*tx;
			ty = 0.0f;
			while (ty <= staticActors[i]->maxBounds.Y) {
				ry = staticActors[i]->org.Y - staticActors[i]->maxBounds.Y + 2.0f*ty;
				int xp = getGridPos(rx, minX, gx);	//calc array pos
				int yp = getGridPos(ry, minY, gy);
				set32BitPixel(pixels, BPP * (xp + yp*w), staticObjCol.R, staticObjCol.G, staticObjCol.B, staticObjCol.A);
				ty += gy*0.5f;	//increase rad by half grid width each time
			}
			tx += gx*0.5f;
		}
	}
}

void ACreateHeatmap::set32BitPixel(uint8 *pixels, int pos, uint8 r, uint8 g, uint8 b, uint8 a){
	pixels[pos + 2] = r;	//draw STATIC obj in grey
	pixels[pos + 1] = g;
	pixels[pos] = b;
	pixels[pos + 3] = a;
}

//
// For TESTING purposes.  Numeric array output in rows and can be viewed in Excel
//
void ACreateHeatmap::outputArrayCSVfile(int w, int h, uint8 * pixels, FString filename){
	FString arrayOut = "Num,Value\n";
	for (int i = 0; i < w*h * BPP; i++)
		arrayOut += FString::Printf(TEXT("%d,%d\n"), i, pixels[i]);
	arrayOut += FString::Printf(TEXT("\nMin,=MIN(B2:B%d)"), w*h * BPP + 1);	//For Excel, output Summary stats
	arrayOut += FString::Printf(TEXT("\nMax,=MAX(B2:B%d)"), w*h * BPP + 1);
	arrayOut += FString::Printf(TEXT("\nAvg,=AVERAGE(B2:B%d)\n"), w*h * BPP + 1);
	FFileHelper::SaveStringToFile(arrayOut, *filename);
}

float ACreateHeatmap::calcDiffInTime() {
	float diffT = totTime - prevTime;	//get LAST pos
	if (diffT > maxTimeAtPos) maxTimeAtPos = diffT;
	return diffT;
}

void ACreateHeatmap::updateLastPositionInArrays(){
	float diffT = calcDiffInTime();
	playerPos[playerPos.Num() - 1].dt = diffT;	//change last array time
	FVector newLocation = player->GetActorLocation();
	playerPositions.RemoveAt(playerPositions.Num() - 1);	//remove last pos
	addPlayerPosToTextArray(newLocation, diffT);
}

void ACreateHeatmap::updatePositionData(FVector &newLocation){
	float diffT = calcDiffInTime();
	PosData p(newLocation.X, newLocation.Y, newLocation.Z, diffT);
	playerPos.Add(p);	//add to dyn array
	addPlayerPosToTextArray(newLocation, diffT);
	prevTime = totTime;
}

void ACreateHeatmap::addPlayerPosToTextArray(FVector &newLocation, float diffT){
	FString d = FString::Printf(TEXT("%f,%f,%f,%f"), newLocation.X, newLocation.Y, newLocation.Z, diffT);
	playerPositions.Add(d);	//add to dyn string array
	allPlayerPos += d + "\r\n";	//append for txt output
}

unsigned int ACreateHeatmap::getGridPos(float rx, float minX, float gx) {
	float epsilon = 0.001f;	//for possible error in calcs
	float px = (rx - minX) / (gx + epsilon);	//calc grid pos
	return (unsigned int)px + 1;
}