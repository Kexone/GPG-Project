#include "stdafx.h"

CubeMap::CubeMap(const char* filenames[6]){
	for (int i = 0; i < 6; i++) {
		map_[i] = LoadTexture(filenames[i], -1);
	}
}


CubeMap::~CubeMap(){
}

Color4 CubeMap::get_texel(Vector3 &direction) {
	int largest = direction.LargestComponent(true);
	int map = 0;
	float dir = 0, u = 0, v = 0;

	switch (largest) {
		case 0: // X
			dir = direction.x;
			if (dir >= 0) { // POSX
				map = 0;
				const float tmp = 1.0f / abs(direction.x);
				u = (direction.y * tmp + 1) * 0.5f;
				v = (direction.z * tmp + 1) * 0.5f;
				u = 1 - u;
			}
			else if (dir < 0) { // NEGX
				map = 1;
				const float tmp = 1.0f / abs(direction.x);
				u = (direction.y * tmp + 1) * 0.5f;
				v = (direction.z * tmp + 1) * 0.5f;
			}
			break;
		case 1: // Y
			dir = direction.y;
			if (dir >= 0) { // POSY
				map = 2;
				const float tmp = 1.0f / abs(direction.y);
				u = (direction.x * tmp + 1) * 0.5f;
				v = (direction.z * tmp + 1) * 0.5f;
			}
			else if (dir < 0) { // NEGY
				map = 3;
				const float tmp = 1.0f / abs(direction.y);
				u = (direction.x * tmp + 1) * 0.5f;
				v = (direction.z * tmp + 1) * 0.5f;
				u = 1 - u;
			}
			break;
		case 2: // Z
			dir = direction.z;
			if (dir >= 0) { // POSZ
				map = 4;
				const float tmp = 1.0f / abs(direction.z);
				u = (direction.x * tmp + 1) * 0.5f;
				v = (direction.y * tmp + 1) * 0.5f;
				v = 1 - v;
			}
			else if (dir < 0) { // NEGZ
				map = 5;
				const float tmp = 1.0f / abs(direction.z);
				u = (direction.x * tmp + 1) * 0.5f;
				v = (direction.y * tmp + 1) * 0.5f;
			}
			break;
		default:
			return Color4(0, 0, 0, 0);
	}

	if (dir == 0 && u == 0 && v == 0 && map == 0) {
		printf("kunda");
	}

	Color4 texel = map_[map]->get_texel(u, v);
	return texel;
	
}
