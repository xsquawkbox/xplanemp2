//
// Created by chris on 13/04/2020.
//

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
