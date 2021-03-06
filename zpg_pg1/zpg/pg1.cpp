#include "stdafx.h"
 int PG1::renderType = 6; // types: 0- object, 1 - sphere , 2-quadric sphere, 3 - cylinder, 4 - ellipsoid, 5 - paraboloid, 6- two part paraboloid

Camera camera;
Ray ray;
CubeMap * map;
std::vector<OmniLight* > lights;
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
				color += BrdfTrace::calcTrace(r, scene, surfaces, c, cubeMap, lights, depth);
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
//#ifndef _DEBUG
//#pragma omp parallel for schedule(dynamic, 9) shared(scene, surfaces, c, image, cubeMap, lights)
//#endif
	clock_t t = clock();
	for (int x = 0; x < image->cols; x++) {
		for (int y = 0; y < image->rows; y++) {
			Color4 color(0.0f, 0.0f, 0.0f, 1.f);
			for (int i = 0; i < count; i++) {
				Ray r = c->GenerateRay(static_cast<float>(x) + positions[i][0], static_cast<float>(y) + positions[i][1]);
				color += RayTrace::rayTrace(r, scene, surfaces, c, cubeMap, lights, depth);
			}
			color /= static_cast<float>(count);
			image->at<cv::Vec3f>(y, x) = cv::Vec3f(color.b, color.g, color.r);
		}
	}
	t = clock() - t;
	printf("\nTime: %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
}

void setScene()
{
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
	//Camera camera = Camera(1280, 720, Vector3(3.0f, 0.f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), DEG2RAD(42.0185f));
	camera = Camera(640, 480, Vector3(6.0f, 0.f, 2.0f), Vector3(0.0f, 0.0f, 0.0f), DEG2RAD(42.0185f));
	//camera = Camera(640, 480, Vector3(-100.0f, 0.f, 4.0f), Vector3(0.0f, 0.0f, 0.0f), DEG2RAD(42.0185f));

	camera.Print();

	ray = camera.GenerateRay(camera.width() / 2.f - 0.5f, camera.height() / 2.f - 0.5f);
	Vector3 v = ray.eval(752.147f);
	printf("Ray: %f, %f, %f", v.x, v.y, v.z);
	//const char* files[] = { "../../data/park/negx.jpg" , "../../data/park/posx.jpg" , "../../data/park/negy.jpg" , "../../data/park/posy.jpg" , "../../data/park/posz.jpg", "../../data/park/negz.jpg" };

	const char* files[] = { "../../data/city/posx.jpg", "../../data/city/negx.jpg", "../../data/city/posy.jpg", "../../data/city/negy.jpg", "../../data/city/posz.jpg", "../../data/city/negz.jpg"};
	map = new CubeMap(files);
	lights = std::vector<OmniLight* >();
	lights.push_back(new OmniLight(Vector3(100.0f, 100.0f, 100.0f), Vector3(0.5, 0.5, 0.5), Vector3(1, 1, 1), Vector3(1, 1, 1)));

}

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
	//if (LoadOBJ("../../data/geosphere.obj", Vector3(1, 1, 1), surfaces, materials) < 0) {
	if (LoadOBJ("../../data/balls/ball_250x250.obj", Vector3(1.f, 1.f, 1.f), surfaces, materials) < 0) {
		//if (LoadOBJ("../../data/136_final.obj", Vector3(1.f, 1.f, 1.f), surfaces, materials) < 0) {
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
	setScene();
	std::string names[7] = { "object" , "sphere" , "quadric sphere","cylinder","ellipsoid", "paraboloid", "two part paraboloid" };
	std::cout << std::endl;
	for (int i = 1; i <7; i++) {
		PG1::renderType = i;
		cv::Mat image = cv::Mat::zeros(camera.height(), camera.width(), CV_32FC3);
		std::cout << std::endl << "Rendering " << names[i] << std::endl;
		render(scene, surfaces, &camera, &image, map, lights, 3);
		//renderBrdf(scene, surfaces, &camera, &reflections, map, lights, 1);
		cv::imshow(names[i], image);
	}

	cv::waitKey();

	rtcDeleteScene(scene); // zrušení Embree scény

	SafeDeleteVectorItems<Material *>(materials);
	SafeDeleteVectorItems<Surface *>(surfaces);

	rtcDeleteDevice(device); // Embree zařízení musíme také uvolnit před ukončením aplikace
	return 0;
}
