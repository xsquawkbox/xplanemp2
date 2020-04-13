//
// Created by chris on 13/04/2020.
//

#include "MapRendering.h"
#include "XPMPMultiplayerVars.h"

XPMPMapRendering::XPMPMapRendering(const std::string &iconSheet, int s, int t, int sheetsize_s, int sheetsize_t):
    mMapSheetPath(iconSheet),
    mThisS(s),
    mThisT(t),
    mSizeS(sheetsize_s),
    mSizeT(sheetsize_t),
    mAircraftLayers{nullptr, nullptr,}
{
    // Create the Aircraft Layers
    for (int ml = 0; ml < ML_COUNT; ml++) {
        const char *mapLayerIdentifier = MapLayerInstanceToString(static_cast<MapLayerInstances>(ml));
        if (XPLMMapExists(mapLayerIdentifier)) {
            tryCreateMapLayers(mapLayerIdentifier, ml);
        }
    }
}

void
XPMPMapRendering::tryCreateMapLayers(const char *mapIdentifier, int position)
{
    XPLMCreateMapLayer_t mapLayerData = {
        sizeof(XPLMCreateMapLayer_t),
        mapIdentifier,
        xplm_MapLayer_Markings,
        nullptr,
        nullptr,
        nullptr,
        &IconCallback,
        &LabelCallback,
        1,
        "Aircraft",
        this
    };
    mAircraftLayers[position] = XPLMCreateMapLayer(&mapLayerData);
}

XPMPMapRendering::~XPMPMapRendering()
{
    for (int ml = 0; ml < ML_COUNT; ml++) {
        if (mAircraftLayers[ml]) {
            XPLMDestroyMapLayer(mAircraftLayers[ml]);
            mAircraftLayers[ml] = nullptr;
        }
    }
}

void
XPMPMapRendering::IconCallback(XPLMMapLayerID inLayer,
                               const float *inMapBoundsLeftTopRightBottom,
                               float zoomRatio,
                               float mapUnitsPerUserInterfaceUnit,
                               XPLMMapStyle mapStyle,
                               XPLMMapProjectionID projection,
                               void *inRefcon)
{
    auto *me = reinterpret_cast<XPMPMapRendering *>(inRefcon);

    me->drawIcons(inLayer,
                  inMapBoundsLeftTopRightBottom,
                  zoomRatio,
                  mapUnitsPerUserInterfaceUnit,
                  mapStyle,
                  projection);
}

void
XPMPMapRendering::LabelCallback(XPLMMapLayerID inLayer,
                                const float *inMapBoundsLeftTopRightBottom,
                                float zoomRatio,
                                float mapUnitsPerUserInterfaceUnit,
                                XPLMMapStyle mapStyle,
                                XPLMMapProjectionID projection,
                                void *inRefcon)
{
    auto *me = reinterpret_cast<XPMPMapRendering *>(inRefcon);

    me->drawLabels(inLayer,
                   inMapBoundsLeftTopRightBottom,
                   zoomRatio,
                   mapUnitsPerUserInterfaceUnit,
                   mapStyle,
                   projection);
}


void
XPMPMapRendering::drawIcons(XPLMMapLayerID inLayer,
                            const float *inMapBoundsLeftTopRightBottom,
                            float zoomRatio,
                            float mapUnitsPerUserInterfaceUnit,
                            XPLMMapStyle mapStyle,
                            XPLMMapProjectionID projection)
{
    if (mMapSheetPath.empty()) {
        return;
    }
    float mapX, mapY;
    for (const auto &aircraftPair: gPlanes) {
        XPLMMapProject(projection,
                       aircraftPair.second->mPosition.lat,
                       aircraftPair.second->mPosition.lon,
                       &mapX,
                       &mapY);

        float iconRotation = XPLMMapGetNorthHeading(projection, mapX, mapY) +
                             aircraftPair.second->mPosition.heading;
        iconRotation = fmod(iconRotation, 360.0f);
        XPLMDrawMapIconFromSheet(inLayer,
                                 mMapSheetPath.c_str(),
                                 mThisS, mThisT,
                                 mSizeS, mSizeT,
                                 mapX,
                                 mapY,
                                 xplm_MapOrientation_Map,
                                 iconRotation,
                                 16);
    }

}

void
XPMPMapRendering::drawLabels(XPLMMapLayerID inLayer,
                             const float *inMapBoundsLeftTopRightBottom,
                             float zoomRatio,
                             float mapUnitsPerUserInterfaceUnit,
                             XPLMMapStyle mapStyle,
                             XPLMMapProjectionID projection)
{
    float mapX, mapY;
    for (const auto &aircraftPair: gPlanes) {
        XPLMMapProject(projection,
                       aircraftPair.second->mPosition.lat,
                       aircraftPair.second->mPosition.lon,
                       &mapX,
                       &mapY);
        XPLMDrawMapLabel(inLayer,
                         aircraftPair.second->mPosition.label,
                         mapX,
                         mapY,
                         xplm_MapOrientation_UI,
                         0);
    }
}
