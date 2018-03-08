//
// Created by kuroneko on 5/03/2018.
//

#include <Carbon/Carbon.h>
#include <XPLMPlugin.h>

#include "AplFSUtil.h"

template <typename T>
struct CFSmartPtr {
	CFSmartPtr(T p) : p_(p) {						  }
	~CFSmartPtr()			 { if (p_) CFRelease(p_); }
	operator T ()			 { return p_; }
	T p_;
};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

int Posix2HFSPath(const char *path, char *result, int resultLen)
{
	CFSmartPtr<CFStringRef>		inStr(CFStringCreateWithCString(kCFAllocatorDefault, path ,kCFStringEncodingMacRoman));
	if (inStr == NULL) return -1;

	CFSmartPtr<CFURLRef>		url(CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLPOSIXPathStyle,0));
	if (url == NULL) return -1;

	CFSmartPtr<CFStringRef>		outStr(CFURLCopyFileSystemPath(url, kCFURLHFSPathStyle));
	if (outStr == NULL) return -1;

	if (!CFStringGetCString(outStr, result, resultLen, kCFStringEncodingMacRoman))
		return -1;

	return 0;
}

int HFS2PosixPath(const char *path, char *result, int resultLen)
{
	bool is_dir = (path[strlen(path)-1] == ':');

	CFSmartPtr<CFStringRef>		inStr(CFStringCreateWithCString(kCFAllocatorDefault, path ,kCFStringEncodingMacRoman));
	if (inStr == NULL) return -1;

	CFSmartPtr<CFURLRef>		url(CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle,0));
	if (url == NULL) return -1;

	CFSmartPtr<CFStringRef>		outStr(CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle));
	if (outStr == NULL) return -1;

	if (!CFStringGetCString(outStr, result, resultLen, kCFStringEncodingMacRoman))
		return -1;

	if(is_dir) strcat(result, "/");

	return 0;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
