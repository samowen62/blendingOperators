#ifndef _INC_H_
#define _INC_H_

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h> 
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <igl/frustum.h>
#include <igl/readOBJ.h>
#include <iostream>
#include <math.h>
#include <assert.h>

#define M_PI            3.14159265358979323846  /* pi */

enum COMPS { 
	UNION_COMP,
	AVERAGE_COMP,
	CLEAN_UNION_COMP,
	BLENDING_COMP,
	INTERPOLATING_COMP
};

#endif