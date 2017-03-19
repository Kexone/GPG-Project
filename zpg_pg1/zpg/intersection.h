#ifndef VSB_GPG_INTERSECT_H
#define VSB_GPG_INTERSECT_H

#include "stdafx.h"

class Intersection
{
public:
	struct SphereArea
	{
		Vector3 center;
		double radius;
	};

	struct QuadricArea {
		union {
			struct {
				float a11;
				float a22;
				float a33;

				float a12;
				float a13;
				float a23;

				float a14;
				float a24;
				float a34;
				float a44;
			};

			float data[10];
		};

		QuadricArea(float a11, float a22, float a33, float a12, float a13, float a23, float a14, float a24, float a34, float a44) :
			a11(a11), a22(a22), a33(a33), a12(a12), a13(a13), a23(a23), a14(a14), a24(a24), a34(a34), a44(a44) {};

		Vector3 getNormal(Vector3 intersectPoint) {
			return Vector3(
				2 * (a11 * intersectPoint.x + a12 * intersectPoint.y + a13 * intersectPoint.z + a14),
				2 * (a22 * intersectPoint.y + a12 * intersectPoint.x + a23 * intersectPoint.z + a24),
				2 * (a33 * intersectPoint.z + a13 * intersectPoint.x + a23 * intersectPoint.y + a34)
			);
		}
	};

	struct Plane {
		Vector3 S;
		Vector3 n;

		Plane(Vector3 S, Vector3 n) : S(S), n(n) {}

		float eval(Vector3 &A, Vector3 &u) {
			float dNU = n.DotProduct(u);
			if (dNU != 0) {
				return n.DotProduct(S - A) / dNU;
			}

			return 0;
		}
	};

	static Ray intersect(Ray& ray, SphereArea sphere_area)
	{
		Vector3 S = sphere_area.center; // sphere
		Vector3 A = ray.org;
		Vector3 u = ray.dir;
		u.Normalize();
		double t1, t2;
		double a = 2 * (A.x * u.x - S.x * u.x +
			A.y * u.y + S.y * u.y +
			A.z * u.z - S.z * u.z);
		double b = 2 * (A.x * u.x + A.y * u.y + A.z * u.z - u.x * S.x - u.y * S.y - u.z * S.z);
		double c = SQR(A.x - S.x) + SQR(A.y - S.y) + SQR(A.z - S.z) - SQR(sphere_area.radius);
		float discriminant = SQR(b) - 4 * c;
		if (discriminant >= 0)
		{
			float disc = sqrt(discriminant);
			ray.tfar = -b / 2.0;
			t1 = (-b + disc) / 2.0;
			if (discriminant != 0)
			{
				t2 = (-b - disc) / 2.0;
				if (t2 > ray.tnear)
				{
					ray.tfar = t2;
				}
				else
				{
					ray.tfar = t1;
				}
				if (t1 < ray.tnear && t2 < ray.tnear)
				{ //pokud se oba trefili za

					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					return ray;
				}

				ray.customIntersector = true;
				ray.collided_normal = ray.eval(ray.tfar) - S;
				ray.collided_normal.Normalize();
				ray.geomID = 0;
				return ray;
			}
		}
		else
		{
			return ray;
		}
		return{};
	}


	static Ray intersectQuadric(Ray& ray, QuadricArea qa)
	{
		Vector3 A = ray.org;
		Vector3 u = ray.dir;
		u.Normalize();
		double t1, t2;

		float a = (qa.a11 * SQR(u.x)) + (qa.a22 * SQR(u.y)) + (qa.a33 * SQR(u.z)) + 2 * ((qa.a12 * u.x * u.y) + (qa.a13 * u.x * u.z) + (qa.a23 * u.y * u.z));
		float b = 2 * ((qa.a11 * A.x * u.x) + (qa.a22 * A.y * u.y) + (qa.a33 * A.z * u.z) + (qa.a12 * A.x * u.y) + (qa.a12 * A.y * u.x) + (qa.a13 * A.x * u.z) + (qa.a13 * A.z * u.x)
			+ (qa.a23 * A.y * u.z) + (qa.a23 * A.z * u.y) + (qa.a14 * u.x) + (qa.a24 * u.y) + (qa.a34 * u.z));
		float c = (qa.a11 * SQR(A.x)) + (qa.a22 * SQR(A.y)) + (qa.a33 * SQR(A.z)) + 2 * ((qa.a12 * A.x * A.y) + (qa.a13 * A.x * A.z) + (qa.a23 * A.y * A.z)
			+ (qa.a14 * A.x) + (qa.a24 * A.y) + (qa.a34 * A.z)) + qa.a44;
		float discriminant = calcDiscriminant(a, b, c);
		if (discriminant >= 0)
		{
			float disc = sqrt(discriminant);
			ray.tfar = -b / (2.0 *a);
			t1 = (-b + disc) / (2.0 *a);
			if (discriminant != 0)
			{
				t2 = (-b - disc) / (2.0 *a);
				if (t2 < t1)
				{
					ray.tfar = t2;
				}
				else
				{
					ray.tfar = t1;
				}
				if (ray.tfar < ray.tnear)
				{ //pokud se oba trefili za

					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					return ray;

				}
				ray.customIntersector = true;
				ray.collided_normal = (qa.getNormal(ray.eval(ray.tfar)));//(qa.getNormal(ray.getIntersectPoint()));
																		 //ray.collided_normal = ray.eval(ray.tfar);
				ray.collided_normal.Normalize();
				ray.geomID = 0;
				return ray;


			}
		}
		else
		{
			return ray;
		}

	};

	static Ray intersectQuadricPlane(Ray& ray, QuadricArea qa) {
		Vector3 A = ray.org;
		Vector3 u = ray.dir;
		u.Normalize();
		double t1, t2;

		// Calc coefficients
		float a = (qa.a11 * SQR(u.x)) + (qa.a22 * SQR(u.y)) + (qa.a33 * SQR(u.z)) + 2 * ((qa.a12 * u.x * u.y) + (qa.a13 * u.x * u.z) + (qa.a23 * u.y * u.z));
		float b = 2 * ((qa.a11 * A.x * u.x) + (qa.a22 * A.y * u.y) + (qa.a33 * A.z * u.z) + (qa.a12 * A.x * u.y) + (qa.a12 * A.y * u.x) + (qa.a13 * A.x * u.z) + (qa.a13 * A.z * u.x)
			+ (qa.a23 * A.y * u.z) + (qa.a23 * A.z * u.y) + (qa.a14 * u.x) + (qa.a24 * u.y) + (qa.a34 * u.z));
		float c = (qa.a11 * SQR(A.x)) + (qa.a22 * SQR(A.y)) + (qa.a33 * SQR(A.z)) + 2 * ((qa.a12 * A.x * A.y) + (qa.a13 * A.x * A.z) + (qa.a23 * A.y * A.z)
			+ (qa.a14 * A.x) + (qa.a24 * A.y) + (qa.a34 * A.z)) + qa.a44;
		float discriminant = calcDiscriminant(a, b, c);
		if (discriminant >= 0)
		{
			Plane planeTop(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f));
			Plane planeBottom(Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f));
			bool usePlaneNormal = false;
			// Calculate plane intersects
			float r1 = planeTop.eval(A, u);
			float r2 = planeBottom.eval(A, u);
			float disc = sqrt(discriminant);
			ray.tfar = -b / (2.0 *a);
			t1 = (-b + disc) / (2.0 *a);
			if (discriminant != 0)
			{
				t2 = (-b - disc) / (2.0 *a);
				if (t2 < t1)
				{
					ray.tfar = t2;
				}
				else
				{
					ray.tfar = t1;
				}
				if (ray.tfar < ray.tnear || r1 < ray.tnear || r2 < ray.tnear) {
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					return ray;
				}

				// Use T based on sort
				if (r1 < r2 && r2 < t1 && t1 < t2) {
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					return ray;
				}
				else if (r1 < t1 && t1 < r2 && r2 < t2) {
					ray.tfar = t1;
				}
				else if (r1 < t1 && t1 < t2 && t2 < r2) {
					ray.tfar = t1;
				}
				else if (t1 < r1 && r1 < t2 && t2 < r2) {
					usePlaneNormal = true;
					ray.tfar = r1;
				}
				else if (t1 < r1 && r1 < r2 && r2 < t2) {
					usePlaneNormal = true;
					ray.tfar = r1;
				}
				else if (t1 < t2 && t2 < r1 && r1 < r2) {
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					return ray;
				}
			}
			ray.customIntersector = true;
			Vector3 normal = usePlaneNormal ? planeTop.n : qa.getNormal(ray.getIntersectPoint());
			ray.collided_normal = normal;//(qa.getNormal(ray.getIntersectPoint()));
																	 //ray.collided_normal = ray.eval(ray.tfar);
			ray.collided_normal.Normalize();
			ray.geomID = 10;
			return ray;

		}
		else
			{
				return ray;
			}

		};

		private:
			static double calcDiscriminant(double a, double b, double c) {
				return SQR(b) - 4 * a * c;
			}
	};

#endif //VSB_GPG_INTERSECT_H
