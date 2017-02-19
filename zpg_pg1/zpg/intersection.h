#pragma once
#include "stdafx.h"
namespace Intersection
{
	struct SphereArea
	{
		Vector3 center;
		double radius;
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
		//double b = SQR(A.x * u.x - S.x * u.x + A.y * u.y + S.y * u.y + A.z * u.z - S.z *u.z);
		double b = 2 * (A.x * u.x + A.y * u.y + A.z * u.z - u.x * S.x - u.y * S.y - u.z * S.z);
		//double c = (SQR(A.x - S.x) + SQR(A.y - S.y) + SQR(A.z - S.z)) * (SQR(u.x));
		double c = SQR(A.x - S.x) + SQR(A.y - S.y) + SQR(A.z - S.z) - SQR(sphere_area.radius);
		float discriminant = SQR(b) - 4 * c;
		if (discriminant >= 0 || discriminant > 0.05) {
			ray.tfar = -b / 2.0;
			t1 = (-b + sqrt(discriminant)) / 2.0;
			if (discriminant != 0) {

				t2 = (-b - sqrt(discriminant)) / 2.0;
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
};

