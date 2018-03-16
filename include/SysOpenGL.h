//
// Created by kuroneko on 2/03/2018.
//

#ifndef SYSOPENGL_H
#define SYSOPENGL_H


#if IBM
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/gl.h>
// #include <GL/glu.h>
#include <GL/glext.h>
#elif APL
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#define glGenVertexArrays(x,y) glGenVertexArraysAPPLE(x,y)
#define glDeleteVertexArrays(x,y) glDeleteVertexArraysAPPLE(x,y)
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glext.h>
#endif

#endif // SYSOPENGL_H
