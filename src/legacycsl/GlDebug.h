//
// Created by kuroneko on 2/03/2018.
//

#ifndef GLDEBUG_H
#define GLDEBUG_H

#ifdef DEBUG_GL

#if IBM
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#elif APL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include "XOGLUtils.h"

void XPMPSetupGLDebug();

#endif

#endif // GLDEBUG_H
