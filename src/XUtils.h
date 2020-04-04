/* 
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef XUTILS_H
#define XUTILS_H

#include <string>
#include <vector>
#include <memory>
#include <XPLMUtilities.h>

#include "XPMPMultiplayer.h"

void	StringToUpper(std::string&);

bool	HasExtNoCase(const std::string& inStr, const char * inExt);

bool    DoesFileExist(const std::string &filePath);

struct XPLMDump {
	XPLMDump() { }

	XPLMDump(const std::string& inFileName, int lineNum, const char * line) {
		XPLMDebugString(XPMP_CLIENT_NAME " WARNING: Parse Error in file ");
		XPLMDebugString(inFileName.c_str());
		XPLMDebugString(" line ");
		char buf[32];
		sprintf(buf,"%d", lineNum);
		XPLMDebugString(buf);
		XPLMDebugString(".\n              ");
		XPLMDebugString(line);
		XPLMDebugString(".\n");
	}

	XPLMDump(const std::string& inFileName, int lineNum, const std::string& line) {
		XPLMDebugString(XPMP_CLIENT_NAME " WARNING: Parse Error in file ");
		XPLMDebugString(inFileName.c_str());
		XPLMDebugString(" line ");
		char buf[32];
		sprintf(buf,"%d", lineNum);
		XPLMDebugString(buf);
		XPLMDebugString(".\n              ");
		XPLMDebugString(line.c_str());
		XPLMDebugString(".\n");
	}

	XPLMDump& operator<<(const char * rhs) {
		XPLMDebugString(rhs);
		return *this;
	}
	XPLMDump& operator<<(const std::string& rhs) {
		XPLMDebugString(rhs.c_str());
		return *this;
	}
	XPLMDump& operator<<(int n) {
		char buf[255];
		sprintf(buf, "%d", n);
		XPLMDebugString(buf);
		return *this;
	}
	XPLMDump& operator<<(size_t n) {
		char buf[255];
		sprintf(buf, "%u", static_cast<unsigned>(n));
		XPLMDebugString(buf);
		return *this;
	}
};

#endif
