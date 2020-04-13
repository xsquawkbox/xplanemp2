/*
 * Copyright (c) 2020, Chris Collins.
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

#ifndef MAPRENDERING_H
#define MAPRENDERING_H

#include <cassert>
#include <string>
#include <XPLMMap.h>

class XPMPMapRendering {
public:
    static void Init();
    static void Shutdown();

    static void ConfigureIcon(const std::string &iconSheet,
                              int s = 0,
                              int t = 0,
                              int sheetsize_s = 1,
                              int sheetsize_t = 1,
                              float iconSize = 35.0f);

protected:
    enum MapLayerInstances {
        ML_UserInterface = 0,
        ML_IOS,
        ML_COUNT
    };

    static const char *MapLayerInstanceToString(MapLayerInstances ml)
    {
        switch (ml) {
        case ML_UserInterface:
            return XPLM_MAP_USER_INTERFACE;
        case ML_IOS:
            return XPLM_MAP_IOS;
        default:
            assert(false);
            return "";
        }
    }

    static void tryCreateMapLayers(const char *mapIdentifier, int position);

    static XPLMMapLayerID gAircraftLayers[ML_COUNT];
    static std::string     gMapSheetPath;
    static int             gThisS, gThisT;
    static int             gSizeS, gSizeT;
    static float           gIconScale;

    static void IconCallback(
        XPLMMapLayerID inLayer,
        const float *inMapBoundsLeftTopRightBottom,
        float zoomRatio,
        float mapUnitsPerUserInterfaceUnit,
        XPLMMapStyle mapStyle,
        XPLMMapProjectionID projection,
        void *inRefcon);

    static void LabelCallback(
        XPLMMapLayerID inLayer,
        const float *inMapBoundsLeftTopRightBottom,
        float zoomRatio,
        float mapUnitsPerUserInterfaceUnit,
        XPLMMapStyle mapStyle,
        XPLMMapProjectionID projection,
        void *inRefcon);

    static void MapCreatedCallback(
        const char *mapIdentifier,
        void *refcon);

};


#endif //MAPRENDERING_H
