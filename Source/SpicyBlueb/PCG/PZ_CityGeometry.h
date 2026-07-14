#pragma once

#include "CoreMinimal.h"

namespace PZCityGeometry
{
	// Calculate the center of a circle which lies on all three points spanning a triangle.
	FVector2D CalculateCircumcenter(const FVector2D& A, const FVector2D& B, const FVector2D& C);
	
	// Evenly shrink a polygon inwards
	TArray<FVector2D> ShrinkPolygon(const TArray<FVector2D>& Poly, float Inset);
	
	TArray<FVector2D> ClipPolygonToBounds(const TArray<FVector2D>& Poly, const FVector2D& HalfExtent);
	
	bool ClipSegmentToBounds(FVector2D& A, FVector2D& B, const FVector2D& HalfExtent);
} // namespace PZCityGeometry
