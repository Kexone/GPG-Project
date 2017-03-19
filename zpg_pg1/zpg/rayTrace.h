#ifndef VSB_GPG_RAY_TRACE_H
#define VSB_GPG_RAY_TRACE_H
#include "color4.h"
#include "ray.h"
#include <vector>
#include "surface.h"
#include "camera.h"
#include "CubeMap.h"
#include "omnilight.h"
#include "vector3.h"
#include "intersection.h"
#include "objects.h"
#include "pg1.h"

class RayTrace
{
public:
	RayTrace() {};

	static Color4 rayTrace(Ray ray, RTCScene& scene, std::vector<Surface *> & surfaces, Camera* cam, CubeMap* cubeMap, std::vector<OmniLight* > &lights, int depth, bool insidePlastic = false) {
		switch(PG1::renderType)
		{
		case 0:
			rtcIntersect(scene, ray); // find the closest hit of a ray segment with the scene
									  // pri filtrovani funkce rtcIntersect jsou pruseciky prochazeny od nejblizsiho k nejvzdalenejsimu
									  // u funkce rtcOccluded jsou nalezene pruseciky vraceny v libovolnem poradi
			break;
		case 1:
			ray = Intersection::intersect(ray, Objects::getSphere());
			break;
		case 2:
		//	ray = Intersection::intersectQuadric(ray, Objects::getQuadricSphere());
			ray = Intersection::intersectQuadricPlane(ray, Objects::getQuadricSphere());
			break;
		case 3:
			//ray = Intersection::intersectQuadric(ray, Objects::getCylinder());
			ray = Intersection::intersectQuadricPlane(ray, Objects::getCylinder());
			break;
		case 4:
			//ray = Intersection::intersectQuadric(ray, Objects::getEllipsoid());
			ray = Intersection::intersectQuadricPlane(ray, Objects::getEllipsoid());
			break;
		case 5:
			//ray = Intersection::intersectQuadric(ray, Objects::getParaboloid());
			ray = Intersection::intersectQuadricPlane(ray, Objects::getParaboloid());
			break;
		case 6:
			//ray = Intersection::intersectQuadric(ray, Objects::getTwoPartParaboloid());
			ray = Intersection::intersectQuadricPlane(ray, Objects::getTwoPartParaboloid());
			break;
		}
		

		if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
			Vector3 normal;
			Surface * surface = surfaces[ray.geomID];
			Vector2 texture_coord;
			if(PG1::renderType == 0)
			{
				Triangle & triangle = surface->get_triangle(ray.primID);
				normal = triangle.normal(ray.u, ray.v);
				texture_coord = triangle.texture_coord(ray.u, ray.v);
			}
			else
			{
				normal = ray.collided_normal;
				texture_coord = Vector2(ray.u, ray.v);
			}
			Material *mat = surface->get_material();
			// získání souøadnic prùseèíku, normál, texturovacích souøadnic atd.
			const Vector3 p = ray.eval(ray.tnear);
			
			bool shadow = false;// inShadow(scene, p, lights[0]->position - p);
			Color4 lambert = LambertShader::getLambert(p, normal, surface, texture_coord, lights[0]);
			//Color4 phong = PhongShader::getPhong(p, normal, surface, texture_coord, lights[0], ray.dir, !shadow);
			if (depth == 0)
				//return phong;
				return lambert;
			Vector3 rayDirection = Vector3(ray.dir[0], ray.dir[1], ray.dir[2]);
			rayDirection.Normalize();
			Vector3 normalizedNormal = Vector3(normal);
			normalizedNormal.Normalize();
			Ray r = Ray(p, rayDirection.Reflect(normalizedNormal), ray.tnear, ray.tfar);
			//Vector3 geometry_normal = Vector3(ray.Ng); // Ng je nenormalizovaná normála zasaženého trojúhelníka vypoètená nesouhlasnì s pravidlem pravé ruky o závitu
			//geometry_normal.Normalize(); // normála zasaženého trojúhelníka vypoètená souhlasnì s pravidlem pravé ruky o závitu

			//Texture* texture = surface->get_material()->get_texture(0);
			//Color4 texel = Color4(surface->get_material()->diffuse.x, surface->get_material()->diffuse.y, surface->get_material()->diffuse.z, 1);
			//if (texture != nullptr)
			//texel = texture->get_texel(texture_coord.x, texture_coord.y);
			if (mat->get_name().compare("wire_214229166") == 0 || mat->get_name().compare("green_plastic_transparent") == 0 || false) {
				Vector3 dir = ray.getDir();
				//Vector3 reflected = 2 * (normalizedNormal.PosDotProduct(-dir)) * normalizedNormal - (-dir);
				float n1 = IOR_AIR;
				float n2 = IOR_PLASTIC;
				if (insidePlastic) {
					n1 = n2;
					n2 = IOR_AIR;
				}
				insidePlastic = !insidePlastic;
				Vector3 rd = ray.getDir();

				// Calc cos_02
				float cos_02 = (-normalizedNormal).DotProduct(rd);
				if (cos_02 < 0) {
					normalizedNormal = -normalizedNormal;
					cos_02 = (-normalizedNormal).DotProduct(rd);
				}

				// Vector rs
				Vector3 rs = rd - (2 * normalizedNormal.DotProduct(rd)) * normalizedNormal;
				rs.Normalize();

				// Generate reflected ray
				Ray reflectedRay = Ray(ray.getIntersectPoint(), rs, 0.01f);

				// Calc cos_01
				float n_d = n1 / n2;
				float sqrt_d = (1 - SQR(n_d) * (1 - SQR(cos_02)));

				// Absolute reflection
				if (sqrt_d < 0.0f) {
					return rayTrace(r, scene, surfaces, cam, cubeMap, lights, depth - 1) * 1.0f;// *diffuse;
				}

				float cos_01 = sqrt(sqrt_d);

				// Vector rr
				Vector3 rr = -n_d * rd - (n_d * cos_02 + cos_01) * normalizedNormal;
				rr.Normalize();

				// Vector lr
				Vector3 lr = rr - (2 * normalizedNormal.DotProduct(rr)) * normalizedNormal;
				lr.Normalize();
				lr = -lr; // l => lr

						  // Fresnel
				float Rs = SQR((n1 * cos_02 - n2 * cos_01) / (n1 * cos_02 + n2 * cos_01));
				float Rp = SQR((n1 * cos_01 - n2 * cos_02) / (n1 * cos_01 + n2 * cos_02));
				float R = 0.5f * (Rs + Rp);

				// Calculate coefficients
				float coefReflect = R;
				float coefRefract = 1.0f - coefReflect;

				// Generate refracted ray
				Ray refractedRay = Ray(ray.getIntersectPoint(), lr, 0.01f);

				// Set IOR
				//refractedRay.ior = n2;
				//reflectedRay.ior = n1;
				Color4 refractedColor = rayTrace(refractedRay, scene, surfaces, cam, cubeMap, lights, depth - 1, insidePlastic) * coefRefract;
				Color4 reflectionColor = rayTrace(r, scene, surfaces, cam, cubeMap, lights, depth - 1) * coefReflect;
				return (reflectionColor + refractedColor);// *texel;
			}

			//return 0.5f * phong + (shadow ? 0.1f : 1.0f) * rayTrace(r, scene, surfaces, cam, cubeMap, lights, --depth) * surface->get_material()->reflectivity * Color4(surface->get_material()->specular, 1) * 0.5f;
			//return 0.5f * phong + rayTrace(r, scene, surfaces, cam, cubeMap, lights, --depth) * surface->get_material()->reflectivity * Color4(surface->get_material()->specular, 1) * 0.5f;
			return 0.5f * lambert + rayTrace(r, scene, surfaces, cam, cubeMap, lights, --depth) * surface->get_material()->reflectivity * Color4(surface->get_material()->specular, 1) * 0.5f;
		}
		else {
			Vector3 dir = Vector3(ray.dir);
			Color4 color = cubeMap->get_texel(dir);
			return color;
		}
	}

private:
	bool inShadow(RTCScene& scene, Vector3 origin, Vector3 shadowDir) {
		Vector3 shadowRayDir = Vector3(shadowDir);
		shadowDir.Normalize();
		Ray shadowRay = Ray(origin, shadowRayDir, 0.01f, shadowRayDir.L2Norm());
		rtcOccluded(scene, shadowRay);
		return shadowRay.geomID != RTC_INVALID_GEOMETRY_ID;
	}

};

#endif