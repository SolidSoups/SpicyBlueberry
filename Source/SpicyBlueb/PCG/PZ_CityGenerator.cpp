#include "PZ_CityGenerator.h"

#include "DrawDebugHelpers.h"
#include "PCGComponent.h"
#include "Components/SplineComponent.h"


APZ_CityGenerator::APZ_CityGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(Root);

	PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG"));
	PCG->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateOnDemand;
	
	USplineComponent* SplineComponent = NewObject<USplineComponent>(this);
	SplineComponent->SetupAttachment(GetRootComponent());
	SplineComponent->ComponentTags.Add(FName("PZ_Roads"));
	SplineComponent->RegisterComponent();
}

void APZ_CityGenerator::Regenerate(int32 NewSeed)
{
	MasterSeed = NewSeed;
	RunGeneration();
}

void APZ_CityGenerator::RegenerateCurrent()
{
	RunGeneration();
}

void APZ_CityGenerator::RandomizeSeedAndRegenerate()
{
	MasterSeed = rand();
	RunGeneration();
}

void APZ_CityGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
		RunGeneration();
}

void APZ_CityGenerator::RunGeneration()
{
	FRandomStream Rng(MasterSeed);
	Blocks.Reset();

	// Clear old splines before rebuilding.
	for (USplineComponent* S : RoadSplines)
	{
		if (S) S->DestroyComponent();
	}
	RoadSplines.Reset();
	
	if (PCG)
	{
		PCG->GenerateLocal(true);
	}

	TArray<FVector2D> Points;
	ScatterPoints(Rng, Points);

	if (Points.Num() < 3) return;

	TArray<int32> Triangles;
	Triangulate(Points, Triangles);

	BuildBlocksFromDelaunay(Points, Triangles);
	BuildRoadSplines(Points, Triangles);

	if (bDrawDebug)
	{
		DrawDebugNetwork();
	}
}

void APZ_CityGenerator::ScatterPoints(FRandomStream& Rng, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();

	const int32 MaxAttempts = BlockCount * 30;
	int32 Attempts = 0;

	while (OutPoints.Num() < BlockCount && Attempts < MaxAttempts)
	{
		++Attempts;

		const FVector2D Candidate(
			Rng.FRandRange(-AreaHalfExtent.X, AreaHalfExtent.X),
			Rng.FRandRange(-AreaHalfExtent.Y, AreaHalfExtent.Y));

		bool bTooClose = false;
		for (const FVector2D& P : OutPoints)
		{
			if (FVector2D::DistSquared(P, Candidate) < MinPointSpacing * MinPointSpacing)
			{
				bTooClose = true;
				break;
			}
		}

		if (!bTooClose)
		{
			OutPoints.Add(Candidate);
		}
	}
}

void APZ_CityGenerator::Triangulate(const TArray<FVector2D>& Points, TArray<int32>& OutTriangles) const
{
	// Bowyer-Watson. Deterministic: depends only on the point set (which is
	// seeded), not on any runtime randomness.
	OutTriangles.Reset();

	const int32 N = Points.Num();
	if (N < 3) return;

	// Work on a local copy so we can append the super-triangle vertices.
	TArray<FVector2D> Pts = Points;

	// Super-triangle big enough to contain all points.
	float MinX = Pts[0].X, MinY = Pts[0].Y, MaxX = Pts[0].X, MaxY = Pts[0].Y;
	for (const FVector2D& P : Pts)
	{
		MinX = FMath::Min(MinX, P.X);
		MinY = FMath::Min(MinY, P.Y);
		MaxX = FMath::Max(MaxX, P.X);
		MaxY = FMath::Max(MaxY, P.Y);
	}
	const float DX = MaxX - MinX;
	const float DY = MaxY - MinY;
	const float DMax = FMath::Max(DX, DY) * 10.f + 1000.f;
	const float MidX = (MinX + MaxX) * 0.5f;
	const float MidY = (MinY + MaxY) * 0.5f;

	const int32 S0 = Pts.Add(FVector2D(MidX - 2 * DMax, MidY - DMax));
	const int32 S1 = Pts.Add(FVector2D(MidX, MidY + 2 * DMax));
	const int32 S2 = Pts.Add(FVector2D(MidX + 2 * DMax, MidY - DMax));

	// Triangle list as flat index triplets.
	TArray<int32> Tris;
	Tris.Append({S0, S1, S2});

	auto InCircumcircle = [&](int32 Ai, int32 Bi, int32 Ci, const FVector2D& P) -> bool
	{
		const FVector2D CC = Circumcenter(Pts[Ai], Pts[Bi], Pts[Ci]);
		const float RSq = FVector2D::DistSquared(CC, Pts[Ai]);
		return FVector2D::DistSquared(CC, P) <= RSq + KINDA_SMALL_NUMBER;
	};

	// Insert each real point one at a time.
	for (int32 i = 0; i < N; ++i)
	{
		const FVector2D P = Pts[i];

		// Find triangles whose circumcircle contains P ("bad" triangles).
		TArray<int32> BadEdges; // pairs of vertex indices forming the cavity polygon
		for (int32 t = Tris.Num() - 3; t >= 0; t -= 3)
		{
			const int32 A = Tris[t], B = Tris[t + 1], C = Tris[t + 2];
			if (InCircumcircle(A, B, C, P))
			{
				// Add its three edges to the boundary list, then remove it.
				BadEdges.Append({A, B, B, C, C, A});
				Tris.RemoveAt(t, 3, EAllowShrinking::No);
			}
		}

		// Keep only edges that appear once (the cavity boundary).
		for (int32 e = 0; e < BadEdges.Num(); e += 2)
		{
			const int32 A = BadEdges[e], B = BadEdges[e + 1];
			bool bShared = false;
			for (int32 f = 0; f < BadEdges.Num(); f += 2)
			{
				if (f == e) continue;
				const int32 C = BadEdges[f], D = BadEdges[f + 1];
				if ((A == C && B == D) || (A == D && B == C))
				{
					bShared = true;
					break;
				}
			}
			if (!bShared)
			{
				Tris.Append({A, B, i});
			}
		}
	}

	// Drop any triangle that still references a super-triangle vertex.
	for (int32 t = Tris.Num() - 3; t >= 0; t -= 3)
	{
		const int32 A = Tris[t], B = Tris[t + 1], C = Tris[t + 2];
		if (A >= N || B >= N || C >= N)
		{
			Tris.RemoveAt(t, 3, EAllowShrinking::No);
		}
	}

	OutTriangles = MoveTemp(Tris);
}

void APZ_CityGenerator::BuildBlocksFromDelaunay(const TArray<FVector2D>& Points, const TArray<int32>& Triangles)
{
	const int32 N = Points.Num();
	const FVector Origin = GetActorLocation();

	// For each site, gather the circumcenters of all triangles touching it.
	// Those circumcenters are the Voronoi cell corners.
	TArray<TArray<FVector2D>> CellCorners;
	CellCorners.SetNum(N);

	for (int32 t = 0; t + 2 < Triangles.Num(); t += 3)
	{
		const int32 A = Triangles[t], B = Triangles[t + 1], C = Triangles[t + 2];
		const FVector2D CC = Circumcenter(Points[A], Points[B], Points[C]);
		CellCorners[A].Add(CC);
		CellCorners[B].Add(CC);
		CellCorners[C].Add(CC);
	}

	for (int32 s = 0; s < N; ++s)
	{
		TArray<FVector2D>& Corners = CellCorners[s];
		if (Corners.Num() < 3)
		{
			continue;
		}

		// Sort corners by angle around the site so the polygon is ordered.
		const FVector2D Site = Points[s];
		Corners.Sort([&](const FVector2D& P0, const FVector2D& P1)
		{
			return FMath::Atan2(P0.Y - Site.Y, P0.X - Site.X)
				< FMath::Atan2(P1.Y - Site.Y, P1.X - Site.X);
		});

		// Clip the cell to the city rectangle so edge cells don't stretch.
		const TArray<FVector2D> Clipped = ClipToBounds(Corners, AreaHalfExtent);
		if (Clipped.Num() < 3)
		{
			continue;
		}

		// Shrink inward to open the road gap between neighbouring blocks.
		const TArray<FVector2D> Shrunk = ShrinkPolygon(Clipped, RoadHalfWidth);

		FPZ_Block Block;
		Block.Center = Origin + FVector(Site.X, Site.Y, 0.f);
		Block.Corners.Reserve(Shrunk.Num());
		for (const FVector2D& C2 : Shrunk)
		{
			Block.Corners.Add(Origin + FVector(C2.X, C2.Y, 0.f));
		}
		Blocks.Add(Block);
	}
}

void APZ_CityGenerator::BuildRoadSplines(const TArray<FVector2D>& Points, const TArray<int32>& Triangles)
{
	const FVector Origin = GetActorLocation();

	// A Voronoi edge is the segment between the circumcenters of the two
	// triangles that share a Delaunay edge. Map each undirected Delaunay edge
	// (a,b) to the circumcenters of the triangles using it.
	TMap<uint64, TArray<FVector2D>> EdgeToCircumcenters;

	auto EdgeKey = [](int32 A, int32 B) -> uint64
	{
		const uint32 Lo = (uint32)FMath::Min(A, B);
		const uint32 Hi = (uint32)FMath::Max(A, B);
		return ((uint64)Hi << 32) | (uint64)Lo;
	};

	for (int32 t = 0; t + 2 < Triangles.Num(); t += 3)
	{
		const int32 A = Triangles[t], B = Triangles[t + 1], C = Triangles[t + 2];
		const FVector2D CC = Circumcenter(Points[A], Points[B], Points[C]);

		EdgeToCircumcenters.FindOrAdd(EdgeKey(A, B)).Add(CC);
		EdgeToCircumcenters.FindOrAdd(EdgeKey(B, C)).Add(CC);
		EdgeToCircumcenters.FindOrAdd(EdgeKey(C, A)).Add(CC);
	}

	for (const TPair<uint64, TArray<FVector2D>>& Pair : EdgeToCircumcenters)
	{
		// Only interior edges (shared by two triangles) form a Voronoi segment.
		if (Pair.Value.Num() != 2)
		{
			continue;
		}

		FVector2D P0 = Pair.Value[0];
		FVector2D P1 = Pair.Value[1];

		// Keep roads inside the city rectangle.
		if (!ClipSegmentToBounds(P0, P1, AreaHalfExtent))
		{
			continue;
		}

		if (FVector2D::DistSquared(P0, P1) < 1.f)
		{
			continue;
		}

		USplineComponent* Spline = NewObject<USplineComponent>(this);
		Spline->SetupAttachment(GetRootComponent());
		Spline->RegisterComponent();
		Spline->SetMobility(EComponentMobility::Movable);
		Spline->ClearSplinePoints(false);

		Spline->AddSplinePoint(Origin + FVector(P0.X, P0.Y, 0.f), ESplineCoordinateSpace::World, false);
		Spline->AddSplinePoint(Origin + FVector(P1.X, P1.Y, 0.f), ESplineCoordinateSpace::World, false);

		// Straight roads: linear between the two points.
		Spline->SetSplinePointType(0, ESplinePointType::Linear, false);
		Spline->SetSplinePointType(1, ESplinePointType::Linear, false);
		Spline->UpdateSpline();

		RoadSplines.Add(Spline);
	}
}

void APZ_CityGenerator::DrawDebugNetwork() const
{
	UWorld* World = GetWorld();
	if (!World) return;

	FlushPersistentDebugLines(World);

	const float Z = 50.f;

	for (const FPZ_Block& Block : Blocks)
	{
		const int32 M = Block.Corners.Num();
		if (M < 2) continue;

		for (int32 i = 0; i < M; ++i)
		{
			const FVector P0 = Block.Corners[i] + FVector(0, 0, Z);
			const FVector P1 = Block.Corners[(i + 1) % M] + FVector(0, 0, Z);
			DrawDebugLine(World, P0, P1, FColor::Cyan, true, -1.f, 0, 14.f);
		}

		DrawDebugSphere(World, Block.Center + FVector(0, 0, Z),
		                40.f, 6, FColor::Yellow, true, -1.f, 0, 3.f);
	}

	// Road splines in blue.
	for (const USplineComponent* Spline : RoadSplines)
	{
		if (!Spline) continue;
		const FVector A = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World) + FVector(0, 0, Z);
		const FVector B = Spline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World) + FVector(0, 0, Z);
		DrawDebugLine(World, A, B, FColor::Blue, true, -1.f, 0, 18.f);
	}
}

FVector2D APZ_CityGenerator::Circumcenter(const FVector2D& A, const FVector2D& B, const FVector2D& C)
{
	const float D = 2.f * (A.X * (B.Y - C.Y) + B.X * (C.Y - A.Y) + C.X * (A.Y - B.Y));
	if (FMath::IsNearlyZero(D))
	{
		return (A + B + C) / 3.f;
	}

	const float A2 = A.X * A.X + A.Y * A.Y;
	const float B2 = B.X * B.X + B.Y * B.Y;
	const float C2 = C.X * C.X + C.Y * C.Y;

	const float Ux = (A2 * (B.Y - C.Y) + B2 * (C.Y - A.Y) + C2 * (A.Y - B.Y)) / D;
	const float Uy = (A2 * (C.X - B.X) + B2 * (A.X - C.X) + C2 * (B.X - A.X)) / D;
	return FVector2D(Ux, Uy);
}

TArray<FVector2D> APZ_CityGenerator::ShrinkPolygon(const TArray<FVector2D>& Poly, float Inset)
{
	TArray<FVector2D> Out;
	const int32 N = Poly.Num();
	if (N < 3) return Poly;

	FVector2D Centroid(0, 0);
	for (const FVector2D& P : Poly) Centroid += P;
	Centroid /= N;

	Out.Reserve(N);
	for (const FVector2D& P : Poly)
	{
		FVector2D Dir = (Centroid - P);
		const float Len = Dir.Size();
		if (Len > KINDA_SMALL_NUMBER)
		{
			Dir /= Len;
			const float Move = FMath::Min(Inset, Len * 0.45f);
			Out.Add(P + Dir * Move);
		}
		else
		{
			Out.Add(P);
		}
	}
	return Out;
}

float APZ_CityGenerator::PolygonSignedArea(const TArray<FVector2D>& Poly)
{
	float Area = 0.f;
	const int32 N = Poly.Num();
	for (int32 i = 0; i < N; ++i)
	{
		const FVector2D& P0 = Poly[i];
		const FVector2D& P1 = Poly[(i + 1) % N];
		Area += (P0.X * P1.Y - P1.X * P0.Y);
	}
	return Area * 0.5f;
}

TArray<FVector2D> APZ_CityGenerator::ClipToBounds(const TArray<FVector2D>& Poly, const FVector2D& HalfExtent)
{
	TArray<FVector2D> Output = Poly;

	// Each edge is defined by a test "is point inside?" and an intersection solve.
	// Order: left (x >= -hx), right (x <= hx), bottom (y >= -hy), top (y <= hy).
	const float HX = HalfExtent.X;
	const float HY = HalfExtent.Y;

	auto ClipEdge = [](const TArray<FVector2D>& In, TFunctionRef<bool(const FVector2D&)> Inside,
	                   TFunctionRef<FVector2D(const FVector2D&, const FVector2D&)> Intersect)-> TArray<FVector2D>
	{
		TArray<FVector2D> Out;
		const int32 N = In.Num();
		if (N == 0) return Out;

		for (int32 i = 0; i < N; ++i)
		{
			const FVector2D& Cur = In[i];
			const FVector2D& Prev = In[(i + N - 1) % N];
			const bool bCurIn = Inside(Cur);
			const bool bPrevIn = Inside(Prev);

			if (bCurIn)
			{
				if (!bPrevIn) Out.Add(Intersect(Prev, Cur));
				Out.Add(Cur);
			}
			else if (bPrevIn)
			{
				Out.Add(Intersect(Prev, Cur));
			}
		}
		return Out;
	};

	// Left edge: x >= -HX
	Output = ClipEdge(Output,
	                  [&](const FVector2D& P) { return P.X >= -HX; },
	                  [&](const FVector2D& A, const FVector2D& B)
	                  {
		                  const float T = (-HX - A.X) / (B.X - A.X);
		                  return FVector2D(-HX, A.Y + T * (B.Y - A.Y));
	                  });

	// Right edge: x <= HX
	Output = ClipEdge(Output,
	                  [&](const FVector2D& P) { return P.X <= HX; },
	                  [&](const FVector2D& A, const FVector2D& B)
	                  {
		                  const float T = (HX - A.X) / (B.X - A.X);
		                  return FVector2D(HX, A.Y + T * (B.Y - A.Y));
	                  });

	// Bottom edge: y >= -HY
	Output = ClipEdge(Output,
	                  [&](const FVector2D& P) { return P.Y >= -HY; },
	                  [&](const FVector2D& A, const FVector2D& B)
	                  {
		                  const float T = (-HY - A.Y) / (B.Y - A.Y);
		                  return FVector2D(A.X + T * (B.X - A.X), -HY);
	                  });

	// Top edge: y <= HY
	Output = ClipEdge(Output,
	                  [&](const FVector2D& P) { return P.Y <= HY; },
	                  [&](const FVector2D& A, const FVector2D& B)
	                  {
		                  const float T = (HY - A.Y) / (B.Y - A.Y);
		                  return FVector2D(A.X + T * (B.X - A.X), HY);
	                  });

	return Output;
}

bool APZ_CityGenerator::ClipSegmentToBounds(FVector2D& A, FVector2D& B, const FVector2D& HalfExtent)
{
	// Liang-Barsky segment clip against the axis-aligned city rectangle.
	const float HX = HalfExtent.X;
	const float HY = HalfExtent.Y;

	float T0 = 0.f, T1 = 1.f;
	const float DX = B.X - A.X;
	const float DY = B.Y - A.Y;

	const float P[4] = {-DX, DX, -DY, DY};
	const float Q[4] = {A.X + HX, HX - A.X, A.Y + HY, HY - A.Y};

	for (int32 i = 0; i < 4; ++i)
	{
		if (FMath::IsNearlyZero(P[i]))
		{
			if (Q[i] < 0.f) return false; // parallel and outside
		}
		else
		{
			const float R = Q[i] / P[i];
			if (P[i] < 0.f)
			{
				if (R > T1) return false;
				if (R > T0) T0 = R;
			}
			else
			{
				if (R < T0) return false;
				if (R < T1) T1 = R;
			}
		}
	}

	const FVector2D NA(A.X + T0 * DX, A.Y + T0 * DY);
	const FVector2D NB(A.X + T1 * DX, A.Y + T1 * DY);
	A = NA;
	B = NB;
	return true;
}
