//
// Created by chris on 13/04/2020.
//

#include "MapRendering.h"
#include "XPMPMultiplayerVars.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#include <cstring>

/* map layers */
XPLMMapLayerID XPMPMapRendering::gAircraftLayers[ML_COUNT] = {
    nullptr,
    nullptr,
};

/* icon sheet parameters */
std::string     XPMPMapRendering::gMapSheetPath;
int             XPMPMapRendering::gThisS = 0;
int             XPMPMapRendering::gThisT = 0;
int             XPMPMapRendering::gSizeS = 1;
int             XPMPMapRendering::gSizeT = 1;
float           XPMPMapRendering::gIconScale = 30.0f;

void
XPMPMapRendering::Init()
{
    // bind the map creation callback
    XPLMRegisterMapCreationHook(&MapCreatedCallback, nullptr);

    // Create the Layers for existing maps
    for (int ml = 0; ml < ML_COUNT; ml++) {
        const char *mapLayerIdentifier = MapLayerInstanceToString(static_cast<MapLayerInstances>(ml));
        if (XPLMMapExists(mapLayerIdentifier)) {
            tryCreateMapLayers(mapLayerIdentifier, ml);
        }
    }
}

void
XPMPMapRendering::Shutdown()
{
    for (int ml = 0; ml < ML_COUNT; ml++) {
        if (gAircraftLayers[ml]) {
            XPLMDestroyMapLayer(gAircraftLayers[ml]);
            gAircraftLayers[ml] = nullptr;
        }
    }
}

void
XPMPMapRendering::MapCreatedCallback(const char *mapIdentifier, void *refcon)
{
    if (!strcmp(mapIdentifier, XPLM_MAP_USER_INTERFACE)) {
        tryCreateMapLayers(mapIdentifier, ML_UserInterface);
    } else if (!strcmp(mapIdentifier, XPLM_MAP_IOS)) {
        tryCreateMapLayers(mapIdentifier, ML_IOS);
    }
}

void
XPMPMapRendering::ConfigureIcon(const std::string &iconSheet,
                                int s,
                                int t,
                                int sheetsize_s,
                                int sheetsize_t,
                                float iconSize)
{
    gMapSheetPath = iconSheet;
    gThisS = s;
    gThisT = t;
    gSizeS = sheetsize_s;
    gSizeT = sheetsize_t;
    gIconScale = iconSize;
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
        nullptr
    };
    if (gAircraftLayers[position] == nullptr) {
        gAircraftLayers[position] = XPLMCreateMapLayer(&mapLayerData);
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
    if (gMapSheetPath.empty()) {
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
                                 gMapSheetPath.c_str(),
                                 gThisS, gThisT,
                                 gSizeS, gSizeT,
                                 mapX,
                                 mapY,
                                 xplm_MapOrientation_Map,
                                 iconRotation,
                                 gIconScale * mapUnitsPerUserInterfaceUnit);
    }
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
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    XPLMMapOrientation labelOrientation = xplm_MapOrientation_UI;

    if (!gMapSheetPath.empty()) {
        // calculate the offset.
        /*
         * BUG: Right now the XPLMMaps API doesn't give us a way to build a
         *     transform to rotate the label offset to counteract the map
         *     orientation when the map is running in heading up mode.
         *
         * because of this, we keep the labels in map orientation if we're
         * rendering icons for now.
         */
        float linearOffset = (gIconScale * mapUnitsPerUserInterfaceUnit) / 1.5f;

        offsetX = 0.0f;
        offsetY = -linearOffset;
        labelOrientation = xplm_MapOrientation_Map;
    }

    float mapX, mapY;
    for (const auto &aircraftPair: gPlanes) {
        XPLMMapProject(projection,
                       aircraftPair.second->mPosition.lat,
                       aircraftPair.second->mPosition.lon,
                       &mapX,
                       &mapY);
        XPLMDrawMapLabel(inLayer,
                         aircraftPair.second->mPosition.label,
                         mapX + offsetX,
                         mapY + offsetY,
                         labelOrientation,
                         0);
    }
}