//
// Created by kuroneko on 2/03/2018.
//

#include <cstdlib>
#include <sstream>
#include "SysOpenGL.h"

#include <XPLMUtilities.h>

#include "XPMPMultiplayer.h"
#include "XOGLUtils.h"
#include "GlDebug.h"

using namespace std;

#ifdef DEBUG_GL

static void xpmpKhrDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *param)
{
	std::stringstream	msgOut;
	msgOut << XPMP_CLIENT_NAME << ": GL Debug: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		msgOut << "[ERR] ";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		msgOut << "[DEP] ";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		msgOut << "[UND] ";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		msgOut << "[PORT] ";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		msgOut << "[PERF] ";
		break;
	case GL_DEBUG_TYPE_MARKER:
		msgOut << "*** ";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		msgOut << " -> ";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		msgOut << " <- ";
		break;
	default:
		break;
	}
	msgOut << std::string(message, length) << std::endl;
	XPLMDebugString(msgOut.str().c_str());
}

void XPMPSetupGLDebug()
{
	glDebugMessageCallback(xpmpKhrDebugProc, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);

	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, 0, NULL, GL_TRUE);

	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_MARKER, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	// eat errors.
	while (GL_NO_ERROR != glGetError())
		;
}

#endif
