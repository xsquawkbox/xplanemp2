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
    explicit XPMPMapRendering(const std::string &iconSheet, int s = 0, int t = 0, int sheetsize_s = 1, int sheetsize_t = 1);

    virtual ~XPMPMapRendering();

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

    void tryCreateMapLayers(const char *mapIdentifier, int position);

    XPLMMapLayerID mAircraftLayers[ML_COUNT];
    std::string     mMapSheetPath;
    int             mThisS, mThisT;
    int             mSizeS, mSizeT;

    static void IconCallback(
        XPLMMapLayerID inLayer,
        const float *inMapBoundsLeftTopRightBottom,
        float zoomRatio,
        float mapUnitsPerUserInterfaceUnit,
        XPLMMapStyle mapStyle,
        XPLMMapProjectionID projection,
        void *inRefcon);

    void drawIcons(XPLMMapLayerID inLayer,
                   const float *inMapBoundsLeftTopRightBottom,
                   float zoomRatio,
                   float mapUnitsPerUserInterfaceUnit,
                   XPLMMapStyle mapStyle,
                   XPLMMapProjectionID projection);

    static void LabelCallback(
        XPLMMapLayerID inLayer,
        const float *inMapBoundsLeftTopRightBottom,
        float zoomRatio,
        float mapUnitsPerUserInterfaceUnit,
        XPLMMapStyle mapStyle,
        XPLMMapProjectionID projection,
        void *inRefcon);

    void drawLabels(XPLMMapLayerID inLayer,
                    const float *inMapBoundsLeftTopRightBottom,
                    float zoomRatio,
                    float mapUnitsPerUserInterfaceUnit,
                    XPLMMapStyle mapStyle,
                    XPLMMapProjectionID projection);

};


#endif //MAPRENDERING_H
