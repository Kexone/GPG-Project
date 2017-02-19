#pragma once
class CubeMap {
private:
	Texture* map_[6];
public:
	CubeMap(const char* filenames[6]);
	~CubeMap();
	Color4 get_texel(Vector3 &direction);
};

