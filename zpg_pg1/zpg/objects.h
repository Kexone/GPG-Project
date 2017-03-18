#ifndef VSB_GPG_OBJECTS_H
#define VSB_GPG_OBJECTS_H
#include "intersection.h"


class Objects
{
public:
	Objects() {};
	//gpg::Quadric Shader::quadric = gpg::Quadric(1.0f, 1.0f, 1.0f, 0, 0, 0, 0, 0, 0, -1.0f); // Sphere
	//gpg::Quadric Shader::quadric = gpg::Quadric(1.0f, 1.0f, 0.0f, 0, 0, 0, 0, 0, 0, -1.0f); // Cylinder
	//gpg::Quadric Shader::quadric = gpg::Quadric(1.0f, 0.5f, 1.0f, 0, 0, 0, 0, 0, 0, -1.0f); // Ellipsoid
	//gpg::Quadric Shader::quadric = gpg::Quadric(1.0f, 1.0f, -1.0f, 0, 0, 0, 0, 0, 0, -1.0f); // Paraboloid
																							 //gpg::Quadric Shader::quadric = gpg::Quadric(1.0f, 1.0f, -1.0f, 0, 0, 0, 0, 0, 0, 1.0f); // Two part Paraboloid
	static Intersection::SphereArea getSphere()
	{
		return  Intersection::SphereArea{ Vector3(0.f, 0.f, 0.f), 1.f };
	}

	static Intersection::QuadricArea getQuadricSphere()
	{
		return Intersection::QuadricArea(1.0f, 1.0f, 1.0f, 0, 0, 0, 0, 0, 0, -1.0f);
	}

	static Intersection::QuadricArea getCylinder()
	{
		return Intersection::QuadricArea(1.0f, 1.0f, 0.0f, 0, 0, 0, 0, 0, 0, -1.0f);
	}

	static Intersection::QuadricArea getEllipsoid()
	{
		return Intersection::QuadricArea(1.0f, 0.5f, 1.0f, 0, 0, 0, 0, 0, 0, -1.0f);
	}

	static Intersection::QuadricArea getParaboloid()
	{
		return Intersection::QuadricArea(1.0f, 1.0f, -1.0f, 0, 0, 0, 0, 0, 0, -1.0f);
	}

	static Intersection::QuadricArea getTwoPartParaboloid()
	{
		return Intersection::QuadricArea(1.0f, 1.0f, -1.0f, 0, 0, 0, 0, 0, 0, 1.0f);
	}

};

#endif