#pragma once
#include "stdafx.h"
#include "intersection.h"

#define EXPECT(expr, cmnt) if (expr) { printf("PASSED\t"); } else{ printf("NOT PASSED "); finalTest &= false; } printf(cmnt);printf("\n");


namespace Testing
{
	static bool test_sphere_intersect()
	{
		bool finalTest = true;
		Ray ray = Ray(Vector3(0.f, 0.f, -5.f), Vector3(0.f, 0.f, 1.f), 0.001);
		Intersection::SphereArea sphere_area = Intersection::SphereArea{ Vector3(0.f, 0.f, 0.f), 1.f };
		Intersection::intersect(ray, sphere_area);
		EXPECT(ray.hasCollided() == true, "Collision");
		Vector3 position = ray.eval(ray.tfar);
		EXPECT(ray.hasCollided() == true, "Collision");
		return finalTest;
	}
	static bool test_sphere_not_intersect()
	{
		bool finalTest = true;
		Ray ray = Ray(Vector3(0.f, 0.f, -5.f), Vector3(0.f, 0.f, -1.f), 0.001);
		Intersection::SphereArea sphere_area = Intersection::SphereArea{ Vector3(0.f, 0.f, 0.f), 1.f };
		Intersection::intersect(ray, sphere_area);
		EXPECT(ray.hasCollided() == false, "Not Collision");
		return finalTest;
	}
	static bool test_normal()
	{
		bool finalTest = true;
		Ray ray = Ray(Vector3(0.f, 0.f, -5.f), Vector3(0.f, 0.f, 1.f), 0.001);
		Intersection::SphereArea sphere_area = Intersection::SphereArea{ Vector3(0.f, 0.f, 0.f), 2.f };
		Intersection::intersect(ray, sphere_area);
		Vector3 position = ray.collidedPosition();
		Vector3 normal = ray.collided_normal;
		EXPECT(position.x == 0.f, "Position");
		EXPECT(position.y == 0.f, "Position");
		EXPECT(position.z == -2.f, "Position");
		//EXPECT(normal == Vector3(0.f, 0.f, -1.f), "Normal");
		Ray passThrough = Ray(position, Vector3(0.f, 0.f, 1.f), 0.01f);
		Intersection::intersect(passThrough, sphere_area);
		Vector3 passThroughCollision = passThrough.collidedPosition();
		Vector3 passthroughNormal = passThrough.collided_normal;
		EXPECT(passThroughCollision.x == 0.f, "Passthrough colision");
		EXPECT(passThroughCollision.y == 0.f, "Passthrough colision");
		EXPECT(passThroughCollision.z == 2.f, "Passthrough colision");
		//EXPECT(passThroughCollision == Vector3(0.f, 0.f, 2.f), "Passthrough colision");
		EXPECT(passthroughNormal.x == 0.f, "Passthrough normal colision");
		EXPECT(passthroughNormal.y == 0.f, "Passthrough normal colision");
		EXPECT(passthroughNormal.z == 1.f, "Passthrough normal colision");
		//EXPECT(passthroughNormal == Vector3(0.f, 0.f, 1.f), "Passthrough normal colision");

		Ray reflected = Ray(position, Vector3(0.f, 0.f, -1.f), 0.01f);
		Intersection::intersect(reflected, sphere_area);
		EXPECT(reflected.hasCollided() == false, "Reflection collision");
		return finalTest;
	}



	static bool testAll()
	{
		bool result = true;
		result &= test_sphere_intersect();
		result &= test_sphere_not_intersect();
		result &= test_normal();

		return result;
	}
}