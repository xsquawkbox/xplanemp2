/* 
 * Copyright (c) 2006, Laminar Research.
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
#ifndef TEXUTILS_H
#define TEXUTILS_H

#include "BitmapUtils.h"
#include <string>

bool LoadTextureFromFile(const std::string &inFileName, bool magentaAlpha, bool inWrap, bool inMipmap, int inDeres, int * outTexNum, int * outWidth, int * outHeight);

bool LoadImageFromFile(const std::string &inFileName, bool magentaAlpha, int inDeres, ImageInfo &im, int * outWidth, int * outHeight);

/* LoadTextureFromMemory reads image data from im and loads it as OpenGL texture. If texNum is 0, a new
   texture id will be allocated automatically, otherwise the texture with the given id will be overwritten. */
bool LoadTextureFromMemory(ImageInfo &im, bool magentaAlpha, bool inWrap, bool mipmap, int &texNum);

#endif
