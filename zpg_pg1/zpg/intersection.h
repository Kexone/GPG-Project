#ifndef VSB_GPG_INTERSECT_H
#define VSB_GPG_INTERSECT_H

#include "stdafx.h"
namespace Intersection
{
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

		static Ray intersect(Ray& ray, SphereArea sphere_area) {
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
			if (discriminant >= 0) {
				float disc = sqrt(discriminant);
				ray.tfar = -b / 2.0;
				t1 = (-b + disc) / 2.0;
				if (discriminant != 0) {

					t2 = (-b - disc) / 2.0;
					if (t2 > ray.tnear) {

						ray.tfar = t2;
					}
					else {

						ray.tfar = t1;
					}
					if (t1 < ray.tnear && t2 < ray.tnear) { //pokud se oba trefili za

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
			else {

				return ray;
			}
		}


		static Ray intesectQuadric(Ray& ray, Intersection::QuadricArea qa)
		{
			Vector3 A = ray.org;
			Vector3 u = ray.dir;
			u.Normalize();
			double t1, t2;

			double a = qa.a11 * SQR(A.x) + qa.a22 * SQR(A.y) + qa.a33 * SQR(A.z) + 2 * qa.a12 * A.x * A.y + 2 * qa.a13 * A.x * A.z + 2 * qa.a23 * A.y * A.z + 2 * qa.a14 * A.x + 2 * qa.a24 * A.y + 2 * qa.a34 * A.z + qa.a44;
			double b = 2 * (qa.a11 * A.x * u.x + qa.a22 * A.y * u.y + qa.a33 * A.z * u.z + qa.a12 * A.x * u.y + qa.a12 * A.y * u.x + qa.a13 * A.x * u.z + qa.a13 * A.z * u.x + qa.a23 * A.y * u.z + qa.a23 * A.z * u.y + qa.a14 * u.x + qa.a24 * u.y + qa.a34 * u.z);
			double c = qa.a11 * SQR(u.x) + qa.a22 * SQR(u.y) + qa.a33 * SQR(u.z) + 2 * qa.a12 * u.x * u.y + 2 * qa.a13 * u.x * u.z + 2 * qa.a23 * u.y * u.z;

			float discriminant = SQR(b) - 4 * c;
			if (discriminant >= 0) {
				float disc = sqrt(discriminant);
				ray.tfar = -b / 2.0;
				t1 = (-b + disc) / 2.0;
				if (discriminant != 0) {

					t2 = (-b - disc) / 2.0;
					if (t2 > ray.tnear) {

						ray.tfar = t2;
					}
					else {

						ray.tfar = t1;
					}
					if (t1 < ray.tnear && t2 < ray.tnear) { //pokud se oba trefili za

						ray.geomID = RTC_INVALID_GEOMETRY_ID;
						return ray;
					}

					ray.customIntersector = true;
					ray.collided_normal = ray.eval(ray.tfar);
					ray.collided_normal.Normalize();
					ray.geomID = 0;
					return ray;
				}
			}
			else {

				return ray;
			}
		};


	};
}

#endif //VSB_GPG_INTERSECT_H