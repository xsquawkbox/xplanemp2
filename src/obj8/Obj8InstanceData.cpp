/*
 * Copyright (c) 2013, Laminar Research.
 * Copyright (c) 2018,2020, Christopher Collins.
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

#include "Obj8InstanceData.h"
#include <cassert>
#include <XPMPMultiplayerVars.h>

void
Obj8InstanceData::updateInstance(
    CSL *csl,
    double x,
    double y,
    double z,
    double pitch,
    double roll,
    double heading,
    xpmp_LightStatus lights,
    XPLMPlaneDrawState_t *state)
{
    auto *myCSL = dynamic_cast<Obj8CSL *>(csl);
    assert(myCSL != nullptr);

    // determine which instance type we want.
    //FIXME: use lowlod + lights as appropriate.
    Obj8DrawType desiredObj = Obj8DrawType::Solid;
	auto fullRenderDistance = gConfiguration.maxFullAircraftRenderingDistance * 1000.0f;
    if (mDistanceSqr > (fullRenderDistance * fullRenderDistance)) {
        desiredObj = Obj8DrawType::LightsOnly;
        if (!myCSL->hasAttachmentsFor(desiredObj)) {
            desiredObj = Obj8DrawType::LowLevelOfDetail;
            if (!myCSL->hasAttachmentsFor(desiredObj)) {
                desiredObj = Obj8DrawType::Solid;
            }
        }
    }

    // Handle each drawtype individually... (there's only three)
    if (desiredObj == Obj8DrawType::Solid) {
        instancePartsForType(myCSL, Obj8DrawType::Solid);
    } else {
        resetPartsForType(myCSL,Obj8DrawType::Solid);
    }
    if (desiredObj == Obj8DrawType::LowLevelOfDetail) {
        instancePartsForType(myCSL, Obj8DrawType::LowLevelOfDetail);
    } else {
        resetPartsForType(myCSL, Obj8DrawType::LowLevelOfDetail);
    }
    instancePartsForType(myCSL, Obj8DrawType::LightsOnly);

    // build the state objects.
    XPLMDrawInfo_t objPosition = {};

    objPosition.structSize = sizeof(objPosition);
    objPosition.x = x;
    objPosition.y = y;
    objPosition.z = z;
    objPosition.heading = heading;
    objPosition.pitch = pitch;
    objPosition.roll = roll;

    // these must be in the same order as defined by dref_names
    float dataRefValues[] = {
        state->gearPosition,
        state->flapRatio,
        state->spoilerRatio,
        state->speedBrakeRatio,
        state->slatRatio,
        state->wingSweep,
        state->thrust,
        state->yokePitch,
        state->yokeHeading,
        state->yokeRoll,
        static_cast<float>((state->thrust < 0.0) ? 1.0 : 0.0),
        static_cast<float>(lights.taxiLights),
        static_cast<float>(lights.landLights),
        static_cast<float>(lights.bcnLights),
        static_cast<float>(lights.strbLights),
        static_cast<float>(lights.navLights)
    };
    for (auto &instanceSet: mInstances) {
        for (auto &instance: instanceSet) {
            if (instance) {
                XPLMInstanceSetPosition(instance, &objPosition, dataRefValues);
            }
        }
    }
}

void
Obj8InstanceData::resetPartsForType(const Obj8CSL *, Obj8DrawType drawType)
{
    const auto instIdx = static_cast<int>(drawType);
    for (auto &instance: mInstances[instIdx]) {
        XPLMDestroyInstance(instance);
        instance = nullptr;
    }
    mInstances[instIdx].clear();
    mInstanceSetPtrs[instIdx] = nullptr;
}

void
Obj8InstanceData::instancePartsForType(const Obj8CSL *csl,
                                       Obj8DrawType drawType)
{
    auto attSet = csl->getAttachmentsFor(drawType);
    if (attSet == nullptr || attSet->empty()) {
        resetPartsForType(nullptr, drawType);
        return;
    }

    const auto instIdx = static_cast<int>(drawType);

    if (mInstanceSetPtrs[instIdx] != static_cast<const void *>(attSet)) {
        // flush the instances we need to recreate them as the set has changed.
        resetPartsForType(nullptr, drawType);
        mInstances[instIdx].resize(attSet->size());
        mInstanceSetPtrs[instIdx] = static_cast<const void *>(attSet);
    }

    auto &instances = mInstances[instIdx];
    const auto &attachments = *attSet;
    for (unsigned int i = 0; i < attachments.size(); i++) {
        if (instances[i] == nullptr) {
        	auto *objHandle = attachments[i]->getObjectHandle();
        	if (nullptr != objHandle) {
				instances[i] = XPLMCreateInstance(attachments[i]->getObjectHandle(),
					                              Obj8CSL::dref_names);
            }
        }
    }
}

void
Obj8InstanceData::resetModel()
{
    resetPartsForType(nullptr, Obj8DrawType::Solid);
    resetPartsForType(nullptr, Obj8DrawType::LowLevelOfDetail);
    resetPartsForType(nullptr, Obj8DrawType::LightsOnly);
}
