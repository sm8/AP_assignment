// SM

#pragma once

#include "GameFramework/HUD.h"
#include "CrossHairHUD.generated.h"

/**
 * 
 */
UCLASS()
class AP_ASSIGNMENT_API ACrossHairHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	// This will be drawn at the centre of the screen.
	UPROPERTY(EditDefaultsOnly)
		UTexture2D* xHairTexture;
public:
	// Primary draw call for the HUD.
	virtual void DrawHUD() override;

	
	
};







