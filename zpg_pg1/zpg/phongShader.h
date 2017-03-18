#ifndef VSB_GPG_PHONG_SHADER_H
#define VSB_GPG_PHONG_SHADER_H
#include "color4.h"
#include "vector3.h"
#include "surface.h"
#include "vector2.h"
#include "omnilight.h"
#include "texture.h"
#include <opencv2/core/cvdef.h>
#include <cmath>

class PhongShader
{
public:
	PhongShader() {};


	Color4 static getPhong(Vector3 p, Vector3 normal, Surface* surface, Vector2 texture_coord, OmniLight* light, Vector3 eye, bool specularOnly = true) {
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

};

#endif