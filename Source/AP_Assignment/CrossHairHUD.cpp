// SM

#include "AP_Assignment.h"
#include "CrossHairHUD.h"


void ACrossHairHUD::DrawHUD() {
	Super::DrawHUD();

	if (xHairTexture) {
		// Find the centre of our canvas.
		FVector2D ctr(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

		// Offset by half of texture's dimensions for alignment.
		FVector2D xHairPos(ctr.X - (xHairTexture->GetSurfaceWidth() * 0.5f), ctr.Y - (xHairTexture->GetSurfaceHeight() * 0.5f));

		// Draw the xHair at the centre.
		FCanvasTileItem tileItem(xHairPos, xHairTexture->Resource, FLinearColor::White);
		tileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(tileItem);
	}
}









