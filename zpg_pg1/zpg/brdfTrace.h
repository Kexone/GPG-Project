#ifndef VSB_GPG_BRDF_TRACE_H
#define VSB_GPG_BRDF_TRACE_H
#include "color4.h"
#include "ray.h"
#include <embree2/rtcore_scene.h>
#include <vector>
#include "surface.h"
#include "camera.h"
#include "CubeMap.h"
#include "omnilight.h"
#include "vector3.h"
#include "intersection.h"


class BrdfTrace
{
public:
	BrdfTrace() {};
	Color4 static calcTrace(Ray& ray, RTCScene scene, std::vector<Surface*>& surfaces, Camera* camera, CubeMap* cube_map, std::vector<OmniLight*>& lights, int depth) {
		Vector3 dir = ray.getDir();
		//rtcIntersect(scene, ray);
		Intersection::SphereArea sphere_area = Intersection::SphereArea{ Vector3(0.f, 0.f, 0.f), 1.f };
		ray = Intersection::intersect(ray, sphere_area);
		// Check collision
		if (!ray.hasCollided()) {
			//return cube_map != nullptr ? Color4(0,0,0, 1) : Color4(1, 1, 1, 1);
			return cube_map != nullptr ? cube_map->get_texel(dir) : Color4(1, 1, 1, 1);
		}

		Surface* surface = surfaces[0];
		//Triangle& triangle = surface->get_triangle(ray.primID);

		// získání souøadnic prùseèíku, normál, texturovacích souøadnic atd.
		const Vector3 p = ray.eval(ray.tfar);
		//Vector3 normal = triangle.normal(ray.u, ray.v);
		Vector3 normal = ray.collided_normal;
		// Check normal orientation
		if (-dir.DotProduct(normal) < 0.0f) {
			normal = -normal;
		}

		// Get material
		Material *mat = surface->get_material();
		Color4 ambient_color = Color4(mat->ambient, 1.0);
		Color4 diffuse_color = Color4(mat->diffuse, 1.0);
		Color4 specular_color = Color4(mat->specular, 1.0);
		// Check if we're not passing the limit
		if (depth > 0) {
			// Render transparent material
			if (mat->get_name() == "green_plastic_transparent" || true) {
				Vector3 rd = ray.getDir();

				// Switch IOR
				float n1 = ray.ior;
				float n2 = switchEnvIor(n1);

				// Calc cos_02
				float cos_02 = (-normal).DotProduct(rd);
				if (cos_02 < 0) {
					normal = -normal;
					cos_02 = (-normal).DotProduct(rd);
				}

				// Vector rs
				Vector3 rs = rd - (2 * normal.DotProduct(rd)) * normal;
				rs.Normalize();

				// Generate reflected ray
				Ray reflectedRay = Ray(ray.getIntersectPoint(), rs, 0.01f);

				// Calc cos_01
				float n_d = n1 / n2;
				float sqrt_d = 1 - SQR(n_d) * (1 - SQR(cos_02));

				// Absolute reflection
				if (sqrt_d < 0.0f) {
					return calcTrace(reflectedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * 1.0f * getDiffuse(ray, surfaces);
				}

				float cos_01 = sqrt(sqrt_d);

				// Vector rr
				Vector3 rr = -n_d * rd - (n_d * cos_02 + cos_01) * normal;
				rr.Normalize();

				// Vector lr
				Vector3 lr = rr - (2 * normal.DotProduct(rr)) * normal;
				lr.Normalize();
				lr = -lr; // l => lr

						  // Fresnel
				float Rs = SQR((n1 * cos_02 - n2 * cos_01) / (n1 * cos_02 + n2 * cos_01));
				float Rp = SQR((n1 * cos_01 - n2 * cos_02) / (n1 * cos_01 + n2 * cos_02));
				float R = 0.5f * (Rs + Rp);

				// Calculate coefficients
				float coefReflect = R;

				if (Random(0.0f, 1.0f) <= coefReflect) {
					// Reflect
					reflectedRay.ior = ray.ior;

					return calcTrace(reflectedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * getDiffuse(ray, surfaces);
				}
				// Refract
				Ray refractedRay = Ray(ray.getIntersectPoint(), lr, 0.01f);
				refractedRay.ior = n2;

				return calcTrace(refractedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * getDiffuse(ray, surfaces);
			}
}
		return ambient_color;

	}

private:
	float static switchEnvIor(float ior) {
		if (ior == IOR_AIR) {
			return IOR_PLASTIC;
		}

		return IOR_AIR;
	}

	Color4 static getDiffuse(Ray &ray, std::vector<Surface*>& surfaces) {
		// Get triangle
		Surface* surface = surfaces.at(ray.geomID);
		Material* material = surface->get_material();
		//Triangle triangle = surface->get_triangle(ray.primID);

		// Get texture u and v coordinates
		//const Vector2 tuv = triangle.texture_coord(ray.u, ray.v);
		const Vector3 tuv = ray.collided_normal;
		// Get texture using id for diffuse slot
		Texture *diff_texture = material->get_texture(Material::kDiffuseMapSlot);
		Color4 diff_color = Color4(material->diffuse, 1);

		// If material has texture return it, otherwise it's material diffuse color
		if (diff_texture != nullptr) {
			diff_color = diff_texture->get_texel(tuv.x, tuv.y);
		}

		return diff_color;
	}

	Color4 calcDiffuse(Ray &ray, Vector3 &normal, OmniLight* light, std::vector<Surface*>& surfaces) {
		// Normal and vector to light from point
		Vector3 toLight = light->position - ray.getIntersectPoint();
		toLight.Normalize();

		// Get dot product between two vectors
		float dot = normal.PosDotProduct(toLight);

		return clamp(dot, 0.0f, 1.0f) * getDiffuse(ray, surfaces) + Color4(light->ambient, 1);
	}

	float albedo() {
		return 1.0f / static_cast<float>(M_PI); // albedo
	}

	Vector3 genRandomDir(Vector3 &normal) {
		// Gen 2 pseudorandom numbers
		float r1 = Random(0.0f, 1.0f);
		float r2 = Random(0.0f, 1.0f);

		// Calculate vector points
		float x = cos(2 * M_PI * r1) * sqrt(1 - SQR(r2));
		float y = sin(2 * M_PI * r1) * sqrt(1 - SQR(r2));
		float z = r2;

		Vector3 dir(x, y, z);
		dir.Normalize();

		// Check if we're in the right hemisphere
		if (normal.DotProduct(dir) < 0.0f) {
			dir = -dir;
		}

		return dir;
	}

	float PDF() {
		return 1.0f / (2.0f * static_cast<float>(M_PI));
	}

	Vector3 genRandomDirProportional(Vector3 &normal) {
		// Gen 2 pseudorandom numbers
		float r1 = Random(0.0f, 1.0f);
		float r2 = Random(0.0f, 1.0f);

		// Calculate vector points
		float x = cos(2 * M_PI * r1) * sqrt(1 - r2);
		float y = sin(2 * M_PI * r1) * sqrt(1 - r2);
		float z = sqrt(r2);

		Vector3 dir(x, y, z);
		dir.Normalize();

		// Check if we're in the right hemisphere
		if (normal.DotProduct(dir) < 0.0f) {
			dir = -dir;
		}

		return dir;
	}

	float PDFProportional(float cosfi) {
		return cosfi / static_cast<float>(M_PI);
	}

	Vector3 getReflectedDir(Vector3 &normal, Vector3 &dir) {
		return 2 * (normal.PosDotProduct(-dir)) * normal - (-dir);
	}

	float PDFMirror() {
		return 1.0f;
	}


	float calcSpecular(Ray &ray, Vector3 &normal, float exp, Camera* camera, OmniLight* light) {
		Vector3 position = ray.getIntersectPoint();

		// Get vector to light
		Vector3 toLight = light->position - position;
		toLight.Normalize();

		// Get vector to camera/eye
		Vector3 toCamera = camera->view_from() - ray.getDir();
		toCamera.Normalize();

		// Calculate Half vector
		Vector3 half = toCamera + toLight;
		half.Normalize();

		// Power of result dot product
		return pow(clamp(half.PosDotProduct(normal), 0.0f, 1.0f), exp);
	}

};

#endif