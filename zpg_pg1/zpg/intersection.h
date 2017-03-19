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

	struct QuadricArea	{
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
			return {};
		}


		static Ray intersectQuadric(Ray& ray, QuadricArea q)
		{
			Vector3 A = ray.org;
			Vector3 u = ray.dir;
			u.Normalize();
			double t1, t2;

			float a = (q.a11 * SQR(u.x)) + (q.a22 * SQR(u.y)) + (q.a33 * SQR(u.z)) + 2 * ((q.a12 * u.x * u.y) + (q.a13 * u.x * u.z) + (q.a23 * u.y * u.z));
			float b = 2 * ((q.a11 * A.x * u.x) + (q.a22 * A.y * u.y) + (q.a33 * A.z * u.z) + (q.a12 * A.x * u.y) + (q.a12 * A.y * u.x) + (q.a13 * A.x * u.z) + (q.a13 * A.z * u.x)
				+ (q.a23 * A.y * u.z) + (q.a23 * A.z * u.y) + (q.a14 * u.x) + (q.a24 * u.y) + (q.a34 * u.z));
			float c = (q.a11 * SQR(A.x)) + (q.a22 * SQR(A.y)) + (q.a33 * SQR(A.z)) + 2 * ((q.a12 * A.x * A.y) + (q.a13 * A.x * A.z) + (q.a23 * A.y * A.z)
				+ (q.a14 * A.x) + (q.a24 * A.y) + (q.a34 * A.z)) + q.a44;
			float discriminant = SQR(b) - (4.0f * a * c);
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
					ray.collided_normal = (q.getNormal(ray.eval(ray.tfar)));//(qa.getNormal(ray.getIntersectPoint()));
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
	};

#endif //VSB_GPG_INTERSECT_H
