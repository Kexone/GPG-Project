#include "stdafx.h"

void rtc_error_function(const RTCError code, const char * str)
{
	printf("ERROR in Embree: %s\n", str);
	exit(1);
}

RTCError check_rtc_or_die(RTCDevice & device)
{
	const RTCError error = rtcDeviceGetError(device);

	if (error != RTC_NO_ERROR)
	{
		printf("ERROR in Embree: ");

		switch (error)
		{
		case RTC_UNKNOWN_ERROR:
			printf("An unknown error has occurred.");
			break;

		case RTC_INVALID_ARGUMENT:
			printf("An invalid argument was specified.");
			break;

		case RTC_INVALID_OPERATION:
			printf("The operation is not allowed for the specified object.");
			break;

		case RTC_OUT_OF_MEMORY:
			printf("There is not enough memory left to complete the operation.");
			break;

		case RTC_UNSUPPORTED_CPU:
			printf("The CPU is not supported as it does not support SSE2.");
			break;

		case RTC_CANCELLED:
			printf("The operation got cancelled by an Memory Monitor Callback or Progress Monitor Callback function.");
			break;
		}

		fflush(stdout);
		exit(1);
	}

	return error;
}

// struktury pro ukládání dat pro Embree
namespace embree_structs
{
	struct Vertex { float x, y, z, a; };
	typedef Vertex Normal;
	struct Triangle { int v0, v1, v2; };
};

void filter_intersection(void * user_ptr, Ray & ray)
{
	/*  All hit information inside the ray is valid.
	The filter function can reject a hit by setting the geomID member of the ray to
	RTC_INVALID_GEOMETRY_ID, otherwise the hit is accepted.The filter function is not
	allowed to modify the ray input data (org, dir, tnear, tfar), but can modify
	the hit data of the ray( u, v, Ng, geomID, primID ). */

	Surface * surface = reinterpret_cast<Surface *>(user_ptr);
	printf("intersection of: %s, ", surface->get_name().c_str());
	const Vector3 p = ray.eval(ray.tfar);
	printf("at: %0.3f (%0.3f, %0.3f, %0.3f)\n", ray.tfar, p.x, p.y, p.z);

	ray.geomID = RTC_INVALID_GEOMETRY_ID; // reject hit
}

void filter_occlusion(void * user_ptr, Ray & ray)
{
	/*  All hit information inside the ray is valid.
	The filter function can reject a hit by setting the geomID member of the ray to
	RTC_INVALID_GEOMETRY_ID, otherwise the hit is accepted.The filter function is not
	allowed to modify the ray input data (org, dir, tnear, tfar), but can modify
	the hit data of the ray( u, v, Ng, geomID, primID ). */

	Surface * surface = reinterpret_cast<Surface *>(user_ptr);
	printf("occlusion of: %s, ", surface->get_name().c_str());
	const Vector3 p = ray.eval(ray.tfar);
	printf("at: %0.3f (%0.3f, %0.3f, %0.3f)\n", ray.tfar, p.x, p.y, p.z);

	ray.geomID = RTC_INVALID_GEOMETRY_ID; // reject hit
}

int test(RTCScene & scene, std::vector<Surface *> & surfaces)
{
	// --- test rtcIntersect -----
	Ray rtc_ray = Ray(Vector3(-1.0f, 0.1f, 0.2f), Vector3(2.0f, 0.0f, 0.0f), 0.0f);
	//Ray rtc_ray = Ray( Vector3( 4.0f, 0.1f, 0.2f ), Vector3( -1.0f, 0.0f, 0.0f ) );
	//rtc_ray.tnear = 0.6f;
	//rtc_ray.tfar = 2.0f;
	rtcIntersect(scene, rtc_ray); // find the closest hit of a ray segment with the scene
								  // pri filtrovani funkce rtcIntersect jsou pruseciky prochazeny od nejblizsiho k nejvzdalenejsimu
								  // u funkce rtcOccluded jsou nalezene pruseciky vraceny v libovolnem poradi

	if (rtc_ray.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		Surface * surface = surfaces[rtc_ray.geomID];
		Triangle & triangle = surface->get_triangle(rtc_ray.primID);
		//Triangle * triangle2 = &( surface->get_triangles()[rtc_ray.primID] );

		// získání souřadnic průsečíku, normál, texturovacích souřadnic atd.
		const Vector3 p = rtc_ray.eval(rtc_ray.tfar);
		Vector3 geometry_normal = -Vector3(rtc_ray.Ng); // Ng je nenormalizovaná normála zasaženého trojúhelníka vypočtená nesouhlasně s pravidlem pravé ruky o závitu
		geometry_normal.Normalize(); // normála zasaženého trojúhelníka vypočtená souhlasně s pravidlem pravé ruky o závitu
		const Vector3 normal = triangle.normal(rtc_ray.u, rtc_ray.v);
		const Vector2 texture_coord = triangle.texture_coord(rtc_ray.u, rtc_ray.v);

		printf("");
	}

	// --- test rtcOccluded -----
	rtc_ray = Ray(Vector3(-1.0f, 0.1f, 0.2f), Vector3(1.0f, 0.0f, 0.0f));
	//rtc_ray.tfar = 1.5;	
	rtcOccluded(scene, rtc_ray); // determining if any hit between a ray segment and the scene exists
								 // po volání rtcOccluded je nastavena pouze hodnota geomID, ostatni jsou nezměněny
	if (rtc_ray.geomID == 0)
	{
		// neco jsme nekde na zadaném intervalu (tnear, tfar) trefili, ale nevime co ani kde
	}

	return 0;
}

cv::Vec3f normalShader(Vector3 normal) {
	Vector3 result = normal / 2.f;
	result.x += 0.5f;
	result.y += 0.5f;
	result.z += 0.5f;
	return cv::Vec3f(result.z, result.y, result.x);
}

Color4 lambertShader(Vector3 p, Vector3 normal, Surface* surface, Vector2 texture_coord, OmniLight* light) {
	Vector3 surface2light = light->position - p;
	surface2light.Normalize();
	Texture* texture = surface->get_material()->get_texture(0);
	Color4 texel = Color4(surface->get_material()->diffuse.x, surface->get_material()->diffuse.y, surface->get_material()->diffuse.z, 1);
	if (texture != nullptr)
		texel = texture->get_texel(texture_coord.x, texture_coord.y);
	Vector3 result = surface2light.DotProduct(normal) * light->diffuse * Vector3(texel.r, texel.g, texel.b);
	return Color4(result, 1);
}

Color4 phongShader(Vector3 p, Vector3 normal, Surface* surface, Vector2 texture_coord, OmniLight* light, Vector3 eye, bool specularOnly = true) {
	normal.Normalize();
	Vector3 toLight = light->position - p;
	toLight.Normalize();

	// Get vector to camera/eye
	Vector3 toCamera = eye - p;
	toCamera.Normalize();

	// Calculate Half vector
	Vector3 half = toCamera + toLight;
	half.Normalize();

	Texture* texture = surface->get_material()->get_texture(0);
	Color4 texel = Color4(surface->get_material()->diffuse.x, surface->get_material()->diffuse.y, surface->get_material()->diffuse.z, 1);
	if (texture != nullptr)
		texel = texture->get_texel(texture_coord.x, texture_coord.y);
	Vector3 finalColor = light->ambient * Vector3(texel.r, texel.g, texel.b);
	float dot_product = MAX(toLight.PosDotProduct(normal), 0.f);
	Vector3 diffuse = dot_product * light->diffuse * Vector3(texel.r, texel.g, texel.b);
	float specularTerm = pow(MAX(half.PosDotProduct(normal), 0.f), surface->get_material()->shininess);
	Vector3 specular = light->specular * surface->get_material()->specular * specularTerm;
	finalColor += diffuse + (specularOnly ? specular : Vector3(0, 0, 0));
	return Color4(finalColor, 1);
}

float calcR(float n1, float n2, const Vector3& normal, const Vector3& ray) {
	float theta_2 = (-normal).DotProduct(-ray);
	if (theta_2 < 0) {
		theta_2 = (normal).DotProduct(-ray);
	}
	// sqrt nekdy zaporna
	float toSqrt = 1 - SQR(n1 / n2) * (1 - SQR(theta_2));
	float theta_1 = toSqrt < 0.0f ? 0.0f : sqrt(toSqrt);
	float Rs = (n1*theta_2 - n2*theta_1) / (n1*theta_2 + n2*theta_1);
	Rs = SQR(Rs);
	float Rp = (n1*theta_1 - n2*theta_2) / (n1*theta_1 + n2*theta_2);
	Rp = SQR(Rp);
	return 0.5f * (Rs + Rp);
}

bool inShadow(RTCScene& scene, Vector3 origin, Vector3 shadowDir) {
	Vector3 shadowRayDir = Vector3(shadowDir);
	shadowDir.Normalize();
	Ray shadowRay = Ray(origin, shadowRayDir, 0.01f, shadowRayDir.L2Norm());
	rtcOccluded(scene, shadowRay);
	return shadowRay.geomID != RTC_INVALID_GEOMETRY_ID;
}

Color4 rayTrace(Ray ray, RTCScene& scene, std::vector<Surface *> & surfaces, Camera* cam, CubeMap* cubeMap, std::vector<OmniLight* > &lights, int depth, bool insidePlastic = false) {
	Intersection::SphereArea sphere_area = Intersection::SphereArea{ Vector3(0.f, 0.f, 0.f), 1.f };
	ray = Intersection::intersect(ray, sphere_area);
	//rtcIntersect(scene, ray); // find the closest hit of a ray segment with the scene
							  // pri filtrovani funkce rtcIntersect jsou pruseciky prochazeny od nejblizsiho k nejvzdalenejsimu
							  // u funkce rtcOccluded jsou nalezene pruseciky vraceny v libovolnem poradi

	if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
		Surface * surface = surfaces[ray.geomID];
		//Triangle & triangle = surface->get_triangle(ray.primID);
		Material *mat = surface->get_material();
		// získání souřadnic průsečíku, normál, texturovacích souřadnic atd.
		const Vector3 p = ray.eval(ray.tnear);
		//const Vector3 normal = triangle.normal(ray.u, ray.v);
		const Vector3 normal = ray.collided_normal;
		//const Vector2 texture_coord = triangle.texture_coord(ray.u, ray.v);
		const Vector2 texture_coord = Vector2(ray.u, ray.v);
		bool shadow = false;// inShadow(scene, p, lights[0]->position - p);
		Color4 phong = phongShader(p, normal, surface, texture_coord, lights[0], ray.dir, !shadow);
		if (depth == 0)
			return phong;
		Vector3 rayDirection = Vector3(ray.dir[0], ray.dir[1], ray.dir[2]);
		rayDirection.Normalize();
		Vector3 normalizedNormal = Vector3(normal);
		normalizedNormal.Normalize();
		Ray r = Ray(p, rayDirection.Reflect(normalizedNormal), ray.tnear, ray.tfar);
		//Vector3 geometry_normal = Vector3(ray.Ng); // Ng je nenormalizovaná normála zasaženého trojúhelníka vypočtená nesouhlasně s pravidlem pravé ruky o závitu
		//geometry_normal.Normalize(); // normála zasaženého trojúhelníka vypočtená souhlasně s pravidlem pravé ruky o závitu

		Texture* texture = surface->get_material()->get_texture(0);
		Color4 texel = Color4(surface->get_material()->diffuse.x, surface->get_material()->diffuse.y, surface->get_material()->diffuse.z, 1);
		if (texture != nullptr)
			texel = texture->get_texel(texture_coord.x, texture_coord.y);
		if (mat->get_name().compare("wire_214229166") == 0 || mat->get_name().compare("green_plastic_transparent") == 0) {
			Vector3 dir = ray.getDir();
			Vector3 reflected = 2 * (normal.PosDotProduct(-dir)) * normal - (-dir);
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
			return (reflectionColor + refractedColor)  * texel;
		}

		return 0.5f * phong + (shadow ? 0.1f : 1.0f) * rayTrace(r, scene, surfaces, cam, cubeMap, lights, --depth) * surface->get_material()->reflectivity * Color4(surface->get_material()->specular, 1) * 0.5f;
	}
	else {
		Vector3 dir = Vector3(ray.dir);
		Color4 color = cubeMap->get_texel(dir);
		return color;
	}
}


/*Color4 rayTrace(Ray ray, RTCScene& scene, std::vector<Surface *> & surfaces, Camera* c, CubeMap* cubeMap, std::vector<OmniLight* > &lights, int depth) {
rtcIntersect(scene, ray); // find the closest hit of a ray segment with the scene
// pri filtrovani funkce rtcIntersect jsou pruseciky prochazeny od nejblizsiho k nejvzdalenejsimu
// u funkce rtcOccluded jsou nalezene pruseciky vraceny v libovolnem poradi

if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
Surface * surface = surfaces[ray.geomID];
Triangle & triangle = surface->get_triangle(ray.primID);

// získání souřadnic průsečíku, normál, texturovacích souřadnic atd.
const Vector3 p = ray.eval(ray.tfar);
Vector3 geometry_normal = -Vector3(ray.Ng); // Ng je nenormalizovaná normála zasaženého trojúhelníka vypočtená nesouhlasně s pravidlem pravé ruky o závitu
geometry_normal.Normalize(); // normála zasaženého trojúhelníka vypočtená souhlasně s pravidlem pravé ruky o závitu
Vector3 normal = triangle.normal(ray.u, ray.v);
normal.Normalize();
const Vector2 texture_coord = triangle.texture_coord(ray.u, ray.v);
bool shadow = false;// inShadow(scene, p, lights[0]->position - p);

Color4 phong = phongShader(p, normal, surface, texture_coord, lights[0], Vector3(ray.org), !shadow);
Ray r = ray.getReflected(normal);
r.ior = ray.ior;
if (surface->get_material()->get_name().compare("wire_214229166") == 0) {
//if (surface->get_material()->get_name().compare("green_plastic_transparent") == 0) {
Texture* texture = surface->get_material()->get_texture(0);
Color4 texel = Color4(surface->get_material()->diffuse.x, surface->get_material()->diffuse.y, surface->get_material()->diffuse.z, 1);
if (texture != nullptr)
texel = texture->get_texel(texture_coord.x, texture_coord.y);
float n1 = ray.ior;
float n2 = (n1 == IOR_PLASTIC) ? IOR_AIR : IOR_PLASTIC;

Ray refractedRay = ray.getRefracted(normal, n1, n2);

float coefReflect = calcR(n1, n2, normal, ray.getDir());
float coefRefract = 1.0f - coefReflect;

if (depth < 0)
return rayTrace(refractedRay, scene, surfaces, c, cubeMap, lights, depth - 1) * coefRefract * texel;

return phong * rayTrace(r, scene, surfaces, c, cubeMap, lights, depth - 1) * coefReflect + rayTrace(refractedRay, scene, surfaces, c, cubeMap, lights, depth - 1) * coefRefract * texel;
}
r.ior = IOR_AIR;
if (depth > 0)
return 0.5f * phong + (shadow ? 0.1f : 1) * rayTrace(r, scene, surfaces, c, cubeMap, lights, depth - 1) * surface->get_material()->reflectivity * Color4(surface->get_material()->specular, 1) * 0.5f;
return phong;
}
else {
Vector3 dir = Vector3(ray.dir);
Color4 color = cubeMap->get_texel(dir);
return color;
}
}*/


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

Color4 getDiffuse(Ray &ray, std::vector<Surface*>& surfaces) {
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

float switchEnvIor(float ior) {
	if (ior == IOR_AIR) {
		return IOR_PLASTIC;
	}

	return IOR_AIR;
}

Color4 brdfTrace(Ray& ray, RTCScene scene, std::vector<Surface*>& surfaces, Camera* camera, CubeMap* cube_map, std::vector<OmniLight*>& lights, int depth) {
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

	// získání souřadnic průsečíku, normál, texturovacích souřadnic atd.
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
				return brdfTrace(reflectedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * 1.0f * getDiffuse(ray, surfaces);
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

				return brdfTrace(reflectedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * getDiffuse(ray, surfaces);
			}
			// Refract
			Ray refractedRay = Ray(ray.getIntersectPoint(), lr, 0.01f);
			refractedRay.ior = n2;

			return brdfTrace(refractedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * getDiffuse(ray, surfaces);
		}
		/*if (Random(0.0f, 1.0f) < 0.15f) {
			// Specular
			Vector3 reflectedDir = getReflectedDir(normal, dir);
			Ray reflectedRay(ray.getIntersectPoint(), reflectedDir, 0.01f);
			reflectedRay.ior = ray.ior;

			return calcSpecular(ray, normal, 3.0f, camera, lights[0]) * brdfTrace(reflectedRay, scene, surfaces, camera, cube_map, lights, depth - 1) * normal.PosDotProduct(reflectedDir); // Mirror ray dir
		}*/
		// Difuse
		//Vector3 randVec = genRandomDirProportional(normal);
		//Ray randRay(ray.getIntersectPoint(), randVec, 0.01f);
		//randRay.ior = ray.ior;

		//return diffuse_color * brdfTrace(randRay, scene, surfaces, camera, cube_map, lights, depth - 1) * normal.PosDotProduct(randVec) * albedo() * (1.0f / PDFProportional(randVec.DotProduct(normal)));
		//return calcDiffuse(ray, normal, lights[0], surfaces) * brdfTrace(randRay, scene, surfaces, camera, cube_map, lights, depth - 1) * normal.PosDotProduct(randVec) * albedo() * (1.0f / PDFProportional(randVec.DotProduct(normal)));
	}

	//return Color4(lights[0]->ambient, 1);
	return ambient_color;
	/*
	Vector3 rayDir = ray.getDir();
	rtcIntersect(scene, ray);

	// Check collision
	if (!ray.hasCollided()) {
	return (cube_map == nullptr) ? Color4(1, 1, 1, 1) : cube_map->get_texel(rayDir);
	}

	Surface * surface = surfaces[ray.geomID];
	Triangle & triangle = surface->get_triangle(ray.primID);

	// získání souřadnic průsečíku, normál, texturovacích souřadnic atd.
	const Vector3 p = ray.eval(ray.tfar);
	Vector3 normal = triangle.normal(ray.u, ray.v);
	normal.Normalize();

	if (-rayDir.DotProduct(normal) < 0.0f) {
	normal = -normal;
	}

	// Get point and material
	Material *mat = surface->get_material();
	Vector3 toEye = p - rayDir;

	// Generate random vector
	//Vector3 randVec = genRandomDir(normal);
	Vector3 randVecP = genRandomDirProportional(normal); // Uniform ray dir
	Vector3 randVec = getReflectedDir(normal, rayDir); // Mirror ray dir
	Color4 color;
	const Vector2 texture_coord = triangle.texture_coord(ray.u, ray.v);
	// Check if we're not passing the limit
	if (depth > 0 && mat->get_name().compare("green_plastic_transparent") == 0) {
	Ray randRay(p, randVec, 0.01f);
	//color = brdfTrace(randRay, scene, surfaces, camera, cube_map, lights, depth - 1, true) * normal.PosDotProduct(randVec) * albedo() * (1.0f / PDF()); // Jitted ray dir
	//color = brdfTrace(randRay, scene, surfaces, camera, cube_map, lights, depth - 1, true) * normal.PosDotProduct(randVec) * albedo() * (1.0f / PDFProportional(randVec.DotProduct(normal))); // Uniform ray dir
	color = 0.5f * brdfTrace(randRay, scene, surfaces, camera, cube_map, lights, depth - 1, true) + 0.5f * phongShader(p, normal, surface, texture_coord, lights[0], ray.org, true); // Mirror ray dir
	}
	else if (depth > 0) {
	Ray randRay(p, randVecP, 0.01f);
	color = 0.5f * brdfTrace(randRay, scene, surfaces, camera, cube_map, lights, depth - 1, true) + PDF() * normal.PosDotProduct(randVecP) * phongShader(p, normal, surface, texture_coord, lights[0], ray.org, true) * (1.0f / PDFProportional(randVecP.DotProduct(normal))); // Uniform ray dir
	}
	else {
	color = phongShader(p, normal, surface, texture_coord, lights[0], ray.org, true);
	}
	return color;
	*/
}

void renderBrdf(RTCScene& scene, std::vector<Surface *> & surfaces, Camera* c, cv::Mat* image, CubeMap* cubeMap, std::vector<OmniLight *> &lights, int depth) {
	int samples = 1;
#ifndef _DEBUG
#pragma omp parallel for schedule(dynamic, 1) shared(scene, surfaces, c, image, cubeMap, lights)
#endif
	for (int x = 0; x < image->cols; x++) {
		for (int y = 0; y < image->rows; y++) {
			Color4 color(0.0f, 0.0f, 0.0f, 1.f);
			for (int i = 0; i < samples; i++) {
				Ray r = c->GenerateRay(static_cast<float>(x) + Random() - 0.5f, static_cast<float>(y) + Random() - 0.5f);
				color += brdfTrace(r, scene, surfaces, c, cubeMap, lights, depth);
			}
			color /= static_cast<float>(samples);
			image->at<cv::Vec3f>(y, x) = cv::Vec3f(color.b, color.g, color.r);
		}
	}
}

void render(RTCScene& scene, std::vector<Surface *> & surfaces, Camera* c, cv::Mat* image, CubeMap* cubeMap, std::vector<OmniLight *> &lights, int depth) {
	const int count = 9;
	float positions[9][2] = {
		{ -0.5f, -0.5f },
		{ 0.5f, -0.5f },
		{ 0.0f, 0.0f },
		{ 0.5f, 0.5f },
		{ -0.5f, 0.5f },
		{ 0.0f, 0.5f },
		{ -0.5f, 0.0f },
		{ 0.5f, 0.0f },
		{ 0.0f, -0.5f }
	};
#ifndef _DEBUG
#pragma omp parallel for schedule(dynamic, 9) shared(scene, surfaces, c, image, cubeMap, lights)
#endif
	for (int x = 0; x < image->cols; x++) {
		for (int y = 0; y < image->rows; y++) {
			Color4 color(0.0f, 0.0f, 0.0f, 1.f);
			for (int i = 0; i < count; i++) {
				Ray r = c->GenerateRay(static_cast<float>(x) + positions[i][0], static_cast<float>(y) + positions[i][1]);
				color += rayTrace(r, scene, surfaces, c, cubeMap, lights, depth);
			}
			color /= static_cast<float>(count);
			image->at<cv::Vec3f>(y, x) = cv::Vec3f(color.b, color.g, color.r);
		}
	}
}

/*
Seznam úkolů:

1, Doplnit TODO v souboru tracing.cpp.
*/

int main(int argc, char * argv[]) {
	printf("PG1, (c)2011-2016 Tomas Fabian\n\n");
	Testing::testAll();
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // Flush to Zero, Denormals are Zero mode of the MXCSR
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
	RTCDevice device = rtcNewDevice(NULL); // musíme vytvořit alespoň jedno Embree zařízení		
	check_rtc_or_die(device); // ověření úspěšného vytvoření Embree zařízení
	rtcDeviceSetErrorFunction(device, rtc_error_function); // registrace call-back funkce pro zachytávání chyb v Embree	

	std::vector<Surface*> surfaces;
	std::vector<Material*> materials;

	// načtení geometrie
	//if (LoadOBJ( "../../data/6887_allied_avenger.obj", Vector3(0.5f, 0.5f, 0.5f), surfaces, materials) < 0) {
	if (LoadOBJ("../../data/geosphere.obj", Vector3(1, 1, 1), surfaces, materials) < 0) {
		return -1;
	}

	// vytvoření scény v rámci Embree
	RTCScene scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1/* | RTC_INTERPOLATE*/);
	// RTC_INTERSECT1 = enables the rtcIntersect and rtcOccluded functions

	// nakopírování všech modelů do bufferů Embree
	for (std::vector<Surface *>::const_iterator iter = surfaces.begin(); iter != surfaces.end(); ++iter) {
		Surface * surface = *iter;
		unsigned geom_id = rtcNewTriangleMesh(scene, RTC_GEOMETRY_STATIC, surface->no_triangles(), surface->no_vertices());
		rtcSetUserData(scene, geom_id, surface);

		// kopírování samotných vertexů trojúhelníků
		embree_structs::Vertex * vertices = static_cast< embree_structs::Vertex*>(rtcMapBuffer(scene, geom_id, RTC_VERTEX_BUFFER));

		for (int t = 0; t < surface->no_triangles(); ++t) {
			for (int v = 0; v < 3; ++v) {
				embree_structs::Vertex & vertex = vertices[t * 3 + v];

				vertex.x = surface->get_triangles()[t].vertex(v).position.x;
				vertex.y = surface->get_triangles()[t].vertex(v).position.y;
				vertex.z = surface->get_triangles()[t].vertex(v).position.z;
			}
		}

		rtcUnmapBuffer(scene, geom_id, RTC_VERTEX_BUFFER);

		// vytváření indexů vrcholů pro jednotlivé trojúhelníky
		embree_structs::Triangle * triangles = static_cast< embree_structs::Triangle * >(
			rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER));

		for (int t = 0, v = 0; t < surface->no_triangles(); ++t) {
			embree_structs::Triangle & triangle = triangles[t];

			triangle.v0 = v++;
			triangle.v1 = v++;
			triangle.v2 = v++;
		}

		rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	}

	rtcCommit(scene);

	// vytvoření kamery
	float sensor_width = 36; // mm
							 //Vector3 view_from = Vector3(-140.0f, -175.0f, 110.0f);
	Vector3 view_from = Vector3(3.0f, 0.0f, 0.0f);
	Vector3 view_at = Vector3(0.0f, 0.0f, 0.0f);
	//Vector3 view_from = Vector3(0, 0.000000000001, 0);
	//Vector3 view_at = Vector3(0, 0, 0);
	//Camera camera = Camera( 1280, 720, view_from, view_at, DEG2RAD(142.185));
	//Camera camera = Camera(1280, 720, view_from, view_at, DEG2RAD(120));
	//Camera camera = Camera(640, 480, Vector3(0, 0, 0), Vector3(-90.f, 0.f, 0.f), DEG2RAD(40.f));
	Camera camera = Camera(1280, 720, Vector3(3.0f, 0.f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), DEG2RAD(42.0185f));

	camera.Print();

	Ray ray = camera.GenerateRay(camera.width() / 2.f - 0.5f, camera.height() / 2.f - 0.5f);
	//ray.tnear = 0.6f;
	Vector3 v = ray.eval(752.147f);
	printf("Ray: %f, %f, %f", v.x, v.y, v.z);
	const char* files[] = { "../../data/park/negx.jpg" , "../../data/park/posx.jpg" , "../../data/park/negy.jpg" , "../../data/park/posy.jpg" , "../../data/park/posz.jpg", "../../data/park/negz.jpg" };

	//const char* files[] = { "../../data/env_map/posx.jpg", "../../data/env_map/negx.jpg", "../../data/env_map/posy.jpg", "../../data/env_map/negy.jpg", "../../data/env_map/posz.jpg", "../../data/env_map/negz.jpg"};
	CubeMap* map = new CubeMap(files);
	std::vector<OmniLight* > lights = std::vector<OmniLight* >();
	lights.push_back(new OmniLight(Vector3(-200.0f, -200.0f, 1100.0f), Vector3(0.3, 0.3, 0.3), Vector3(1, 1, 1), Vector3(1, 1, 1)));

	cv::Mat reflections = cv::Mat::zeros(camera.height(), camera.width(), CV_32FC3);
	render(scene, surfaces, &camera, &reflections, map, lights, 10);
	//renderBrdf(scene, surfaces, &camera, &reflections, map, lights, 1);
	cv::imshow("Reflections", reflections);
	/*
	cv::Mat no_reflections(camera.height(), camera.width(), CV_32FC3);
	render(scene, surfaces, &camera, &no_reflections, map, lights, 1);
	cv::imshow("No reflections", no_reflections);
	*/
	cv::waitKey();

	rtcDeleteScene(scene); // zrušení Embree scény

	SafeDeleteVectorItems<Material *>(materials);
	SafeDeleteVectorItems<Surface *>(surfaces);

	rtcDeleteDevice(device); // Embree zařízení musíme také uvolnit před ukončením aplikace
	return 0;
}
