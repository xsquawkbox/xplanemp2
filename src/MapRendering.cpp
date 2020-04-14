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

#include "MapRendering.h"
#include "XPMPMultiplayerVars.h"
#include <cmath>

#ifndef M_PI
#warning M_PI is not being defiend by your cmath header.  Fix your compiler defines.
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
    float linearOffset = 0.0f;
    float mapX, mapY;
    double rotation;
    if (!gMapSheetPath.empty()) {
        // calculate the offset.
        linearOffset = -(gIconScale * mapUnitsPerUserInterfaceUnit);
    }

    for (const auto &aircraftPair: gPlanes) {
        XPLMMapProject(projection,
                       aircraftPair.second->mPosition.lat,
                       aircraftPair.second->mPosition.lon,
                       &mapX,
                       &mapY);
        // rotation needs to be in radians for the sin/cos
        if (linearOffset != 0.0f) {
            rotation =
                XPLMMapGetNorthHeading(projection, mapX, mapY) * M_PI / 180.0;

            offsetX = static_cast<float>(-sin(rotation) * linearOffset);
            offsetY = static_cast<float>(cos(rotation) * linearOffset);
        }
        XPLMDrawMapLabel(inLayer,
                         aircraftPair.second->mPosition.label,
                         mapX + offsetX,
                         mapY + offsetY,
                         xplm_MapOrientation_UI,
                         0);
    }
}