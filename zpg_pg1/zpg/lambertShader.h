#ifndef VSB_GPG_LAMBERT_SHADER_H
#define VSB_GPG_LAMBERT_SHADER_H
#include "color4.h"
#include "vector3.h"
#include "surface.h"
#include "vector2.h"
#include "omnilight.h"
#include "texture.h"

class LambertShader
{
public:
	LambertShader() {};


	Color4 static getLambert(Vector3 p, Vector3 normal, Surface* surface, Vector2 texture_coord, OmniLight* light) {
		Vector3 surface2light = light->position - p;
		surface2light.Normalize();
		Texture* texture = surface->get_material()->get_texture(0);
		Color4 texel = Color4(surface->get_material()->diffuse.x, surface->get_material()->diffuse.y, surface->get_material()->diffuse.z, 1);
		if (texture != nullptr)
			texel = texture->get_texel(texture_coord.x, texture_coord.y);
		Vector3 result = surface2light.DotProduct(normal) * light->diffuse * Vector3(texel.r, texel.g, texel.b);
		return Color4(result, 1);
	}
};

#endif