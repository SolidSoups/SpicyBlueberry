#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_CityGenerator.generated.h"

class UPCGComponent;
class USplineComponent;

DECLARE_MULTICAST_DELEGATE(FPZ_OnCityGenerated);

USTRUCT()
struct FPZ_Block
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector Center = FVector::ZeroVector;
	UPROPERTY()
	TArray<FVector> Corners;
};

UCLASS()
class SPICYBLUEB_API APZ_CityGenerator : public AActor
{
	GENERATED_BODY()

public:
	APZ_CityGenerator();
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void Regenerate(int32 NewSeed);
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void RegenerateCurrent();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void RandomizeSeedAndRegenerate();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void ClearCity();

	TArray<FVector> GetRestaurantSpawns(int32 NumPlayers) const;
	TArray<FVector> GetDeliveryCandidates() const;
	
	// Seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Seed")
	int32 MasterSeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Seed")
	bool bGenerateOnBeginPlay = true;
	
	// Road Network
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads", meta = (ClampMin = "3"))
	int32 NumBlocksToMake = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads")
	FVector2D AreaHalfExtent = FVector2D(4000.f, 4000.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads", meta = (ClampMin = "0"))
	float MinPointSpacing = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads")
	float RoadHalfWidth = 150.f;
	
	// Debugging
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Debug")
	bool bDrawDebug = false;

	bool IsCityReady = false;
	FPZ_OnCityGenerated OnCityGenerated;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	UPCGComponent* PCG;
	
	// One spline per road segment (Voronoi edge). PCG reads these via Get Spline Data.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	TArray<USplineComponent*> RoadSplines;

private:
	void RunGeneration();

	// Pipeline steps.
	void GenerateScatteredPoints(int32 NumPoints, FRandomStream& Range, TArray<FVector2D>& OutPoints) const;
	void TriangulatePoints(const TArray<FVector2D>& Points, TArray<int32>& OutTriangles) const;
	void BuildBlocksFromDelaunay(const TArray<FVector2D>& Points, const TArray<int32>& Triangles);
	void BuildRoadSplines(const TArray<FVector2D>& Points, const TArray<int32>& Triangles);
	
	void DrawDebugNetwork() const;
	
	TArray<FPZ_Block> Blocks;
};