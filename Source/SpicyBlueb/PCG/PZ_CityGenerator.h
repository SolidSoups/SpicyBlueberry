#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_CityGenerator.generated.h"

class UPCGComponent;

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
	
	// Seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Seed")
	int32 MasterSeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Seed")
	bool bGenerateOnBeginPlay = true;
	
	// Road Network
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads", meta = (ClampMin = "3"))
	int32 BlockCount = 40;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads")
	FVector2D AreaHalfExtent = FVector2D(4000.f, 4000.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads", meta = (ClampMin = "0"))
	float MinPointSpacing = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City|Roads")
	float RoadHalfWidth = 150.f;
	
	// Buttons
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void Regenerate(int32 NewSeed);
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void RegenerateCurrent();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "City|Seed")
	void RandomizeSeedAndRegenerate();


protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	UPCGComponent* PCG;
	
	TArray<FPZ_Block> Blocks;
	
private:
	void RunGeneration();

	// Pipeline steps.
	void ScatterPoints(FRandomStream& Rng, TArray<FVector2D>& OutPoints) const;
	void Triangulate(const TArray<FVector2D>& Points, TArray<int32>& OutTriangles) const;
	void BuildBlocksFromDelaunay(const TArray<FVector2D>& Points, const TArray<int32>& Triangles);
	
	void DrawDebugNetwork() const;

	// Helpers
	static FVector2D Circumcenter(const FVector2D& A, const FVector2D& B, const FVector2D& C);
	static TArray<FVector2D> ShrinkPolygon(const TArray<FVector2D>& Poly, float Inset);
	static float PolygonSignedArea(const TArray<FVector2D>& Poly);
	static TArray<FVector2D> ClipToBounds(const TArray<FVector2D>& Poly, const FVector2D& HalfExtent);
};
