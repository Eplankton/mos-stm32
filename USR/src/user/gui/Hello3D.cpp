#define GUILITE_ON //Do not define this macro once more!!!

#include "GuiLite.h"
#include "stdint.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define UI_WIDTH   128
#define UI_HEIGHT  160
#define SHAPE_SIZE 25

static c_surface* s_surface;
static c_display* s_display;

// 3D engine
inline void multiply(int m, int n, int p, float* a, float* b, float* c) // a[m][n] * b[n][p] = c[m][p]
{
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < p; j++) {
			float temp = 0;
			for (int k = 0; k < n; k++) {
				temp += a[i * n + k] * b[k * p + j];
			}
			c[i * p + j] = temp;
		}
	}
}

inline void rotateX(float angle, float* point, float* output) // rotate matrix for X
{
	static float rotation[3][3];
	rotation[0][0] = 1;
	rotation[1][1] = cos(angle);
	rotation[1][2] = 0 - sin(angle);
	rotation[2][1] = sin(angle);
	rotation[2][2] = cos(angle);
	multiply(3, 3, 1, (float*) rotation, point, output);
}

inline void rotateY(float angle, float* point, float* output) // rotate matrix for Y
{
	static float rotation[3][3];
	rotation[0][0] = cos(angle);
	rotation[0][2] = sin(angle);
	rotation[1][1] = 1;
	rotation[2][0] = 0 - sin(angle);
	rotation[2][2] = cos(angle);
	multiply(3, 3, 1, (float*) rotation, point, output);
}

inline void rotateZ(float angle, float* point, float* output) // rotate matrix for Z
{
	static float rotation[3][3];
	rotation[0][0] = cos(angle);
	rotation[0][1] = 0 - sin(angle);
	rotation[1][0] = sin(angle);
	rotation[1][1] = cos(angle);
	rotation[2][2] = 1;
	multiply(3, 3, 1, (float*) rotation, point, output);
}

inline void projectOnXY(float* point, float* output, float zFactor = 1)
{
	static float projection[2][3]; //project on X/Y face
	projection[0][0] = zFactor;    //the raio of point.z and camera.z
	projection[1][1] = zFactor;    //the raio of point.z and camera.z
	multiply(2, 3, 1, (float*) projection, point, output);
}

// Shape
class Shape
{
public:
	Shape() { angle = 0.5; }
	virtual void draw(uint16_t x, uint16_t y, bool isErase) = 0;
	virtual void rotate()                                   = 0;

protected:
	float angle;
};

class Cube : public Shape
{
public:
	Cube() { memset(points2d, 0, sizeof(points2d)); }
	virtual void draw(uint16_t x, uint16_t y, bool isErase)
	{
		for (uint8_t i = 0; i < 4; i++) {
			s_surface->draw_line(points2d[i][0] + x, points2d[i][1] + y, points2d[(i + 1) % 4][0] + x, points2d[(i + 1) % 4][1] + y, (isErase) ? 0 : 0xffff0000, Z_ORDER_LEVEL_0);
			s_surface->draw_line(points2d[i + 4][0] + x, points2d[i + 4][1] + y, points2d[((i + 1) % 4) + 4][0] + x, points2d[((i + 1) % 4) + 4][1] + y, (isErase) ? 0 : 0xff00ff00, Z_ORDER_LEVEL_0);
			s_surface->draw_line(points2d[i][0] + x, points2d[i][1] + y, points2d[(i + 4)][0] + x, points2d[(i + 4)][1] + y, (isErase) ? 0 : 0xffffff00, Z_ORDER_LEVEL_0);
		}
	}
	virtual void rotate()
	{
		float rotateOut1[3][1], rotateOut2[3][1], rotateOut3[3][1];
		for (uint8_t i = 0; i < 8; i++) {
			rotateX(angle, points[i], (float*) rotateOut1);
			rotateY(angle, (float*) rotateOut1, (float*) rotateOut2);
			rotateZ(angle, (float*) rotateOut2, (float*) rotateOut3);
			projectOnXY((float*) rotateOut3, (float*) points2d[i]);
		}
		angle += 0.1;
	}

private:
	static float points[8][3];
	float points2d[8][2];
};

float Cube::points[8][3] = {
        {-SHAPE_SIZE, -SHAPE_SIZE, -SHAPE_SIZE}, // x, y, z
        { SHAPE_SIZE, -SHAPE_SIZE, -SHAPE_SIZE},
        { SHAPE_SIZE,  SHAPE_SIZE, -SHAPE_SIZE},
        {-SHAPE_SIZE,  SHAPE_SIZE, -SHAPE_SIZE},
        {-SHAPE_SIZE, -SHAPE_SIZE,  SHAPE_SIZE},
        { SHAPE_SIZE, -SHAPE_SIZE,  SHAPE_SIZE},
        { SHAPE_SIZE,  SHAPE_SIZE,  SHAPE_SIZE},
        {-SHAPE_SIZE,  SHAPE_SIZE,  SHAPE_SIZE}
};

class Pyramid : public Shape
{
public:
	Pyramid() { memset(points2d, 0, sizeof(points2d)); }
	virtual void draw(uint16_t x, uint16_t y, bool isErase)
	{
		s_surface->draw_line(points2d[0][0] + x, points2d[0][1] + y, points2d[1][0] + x,
		                     points2d[1][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
		s_surface->draw_line(points2d[0][0] + x, points2d[0][1] + y, points2d[2][0] + x,
		                     points2d[2][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
		s_surface->draw_line(points2d[0][0] + x, points2d[0][1] + y, points2d[3][0] + x,
		                     points2d[3][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
		s_surface->draw_line(points2d[0][0] + x, points2d[0][1] + y, points2d[4][0] + x,
		                     points2d[4][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);

		s_surface->draw_line(points2d[1][0] + x, points2d[1][1] + y, points2d[2][0] + x,
		                     points2d[2][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
		s_surface->draw_line(points2d[2][0] + x, points2d[2][1] + y, points2d[3][0] + x,
		                     points2d[3][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
		s_surface->draw_line(points2d[3][0] + x, points2d[3][1] + y, points2d[4][0] + x,
		                     points2d[4][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
		s_surface->draw_line(points2d[4][0] + x, points2d[4][1] + y, points2d[1][0] + x,
		                     points2d[1][1] + y, (isErase) ? 0 : 0xff007acc, Z_ORDER_LEVEL_0);
	}

	virtual void rotate()
	{
		float rotateOut1[3][1], rotateOut2[3][1];
		for (int i = 0; i < 5; i++)
		{
			rotateY(angle, points[i], (float*) rotateOut1);
			rotateX(0.1, (float*) rotateOut1, (float*) rotateOut2);
			float zFactor = SHAPE_SIZE / (2.2 * SHAPE_SIZE - rotateOut2[2][0]);
			projectOnXY((float*) rotateOut2, (float*) points2d[i], zFactor);
		}
		angle += 0.1;
	}

private:
	static float points[5][3];
	float points2d[5][2];
};

float Pyramid::points[5][3] = {
        {          0, -SHAPE_SIZE,           0}, // top
        {-SHAPE_SIZE,  SHAPE_SIZE, -SHAPE_SIZE},
        { SHAPE_SIZE,  SHAPE_SIZE, -SHAPE_SIZE},
        { SHAPE_SIZE,  SHAPE_SIZE,  SHAPE_SIZE},
        {-SHAPE_SIZE,  SHAPE_SIZE,  SHAPE_SIZE}
};

// Demo
void create_ui(void* phy_fb, uint16_t screen_width, uint16_t screen_height, int color_bytes, struct EXTERNAL_GFX_OP* gfx_op)
{
	if (phy_fb) {
		static c_surface surface(UI_WIDTH, UI_HEIGHT, color_bytes, Z_ORDER_LEVEL_0);
		static c_display display(phy_fb, screen_width, screen_height, &surface);
		s_surface = &surface;
		s_display = &display;
	}
	else
	{ //for MCU without framebuffer
		static c_surface_no_fb surface_no_fb(UI_WIDTH, UI_HEIGHT, color_bytes, gfx_op, Z_ORDER_LEVEL_0);
		static c_display display(phy_fb, screen_width, screen_height, &surface_no_fb);
		s_surface = &surface_no_fb;
		s_display = &display;
	}
	s_surface->fill_rect(0, 0, UI_WIDTH - 1, UI_HEIGHT - 1, 0, Z_ORDER_LEVEL_0);

	Cube theCube;
	Pyramid thePyramid;

	while (true) {
		theCube.draw(64, 50, true); //erase footprint
		theCube.rotate();
		theCube.draw(64, 50, false); //refresh cube

		thePyramid.draw(64, 120, true); //erase footprint
		thePyramid.rotate();
		thePyramid.draw(64, 120, false); //refresh pyramid

		// thread_sleep(50);
	}
}

//////////////////////// interface for all platform ////////////////////////
extern "C" void startHello3D(void* phy_fb, uint16_t width, uint16_t height, int color_bytes, struct EXTERNAL_GFX_OP* gfx_op)
{
	create_ui(phy_fb, width, height, color_bytes, gfx_op);
}

extern "C" void* getUiOfHello3D(int* width, int* height, bool force_update)
{
	return s_display->get_updated_fb(width, height, force_update);
}