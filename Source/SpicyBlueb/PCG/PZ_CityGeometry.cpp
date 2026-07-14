#include "PZ_CityGeometry.h"

FVector2D PZCityGeometry::CalculateCircumcenter(const FVector2D& A, const FVector2D& B, const FVector2D& C)
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

TArray<FVector2D> PZCityGeometry::ShrinkPolygon(const TArray<FVector2D>& Poly, float Inset)
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

TArray<FVector2D> PZCityGeometry::ClipPolygonToBounds(const TArray<FVector2D>& Poly, const FVector2D& HalfExtent)
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

bool PZCityGeometry::ClipSegmentToBounds(FVector2D& A, FVector2D& B, const FVector2D& HalfExtent)
{
	// Liang-Barsky segment clip against the axis-aligned city rectangle.
	const float HX = HalfExtent.X;
	const float HY = HalfExtent.Y;
	

	float T0 = 0.f, T1 = 1.f;
	const float DX = B.X - A.X;
	const float DY = B.Y - A.Y;

	const float P[4] = {-DX, DX, -DY, DY};
	const float Q[4] = {
		static_cast<float>(A.X) + HX,
		HX - static_cast<float>(A.X),
		static_cast<float>(A.Y) + HY,
		HY - static_cast<float>(A.Y)
	};

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
