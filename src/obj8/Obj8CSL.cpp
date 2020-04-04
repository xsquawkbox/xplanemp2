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

#include "Obj8CSL.h"

#include <string>
#include <deque>
#include <queue>
#include <utility>
#include <XPMPMultiplayerVars.h>
#include <XPLMDataAccess.h>

#include "Obj8InstanceData.h"

using namespace std;


const char * Obj8CSL::dref_names[] = {
	"libxplanemp/controls/gear_ratio",
	"libxplanemp/controls/flap_ratio",
	"libxplanemp/controls/spoiler_ratio",
	"libxplanemp/controls/speed_brake_ratio",
	"libxplanemp/controls/slat_ratio",
	"libxplanemp/controls/wing_sweep_ratio",
	"libxplanemp/controls/thrust_ratio",
	"libxplanemp/controls/yoke_pitch_ratio",
	"libxplanemp/controls/yoke_heading_ratio",
	"libxplanemp/controls/yoke_roll_ratio",
	"libxplanemp/controls/thrust_revers",
	"libxplanemp/controls/taxi_lites_on",
	"libxplanemp/controls/landing_lites_on",
	"libxplanemp/controls/beacon_lites_on",
	"libxplanemp/controls/strobe_lites_on",
	"libxplanemp/controls/nav_lites_on",
	nullptr
};

std::vector<float> Obj8CSL::dref_values;

typedef void (* XPLMSetDataf_f)(
	void *               inRefcon,
	float                inValue);
typedef float (* XPLMGetDataf_f)(
	void *               inRefcon);

float
Obj8CSL::obj8_dref_read(void *inRefcon)
{
	intptr_t idx = reinterpret_cast<intptr_t>(inRefcon);
	if (idx > static_cast<intptr_t>(Obj8CSL::dref_values.size()) || idx < 0) {
		return 0.0f;
	}
	return Obj8CSL::dref_values[idx];
}

void
Obj8CSL::obj8_dref_write(void *inRefcon, float inValue)
{
	intptr_t idx = reinterpret_cast<intptr_t>(inRefcon);
	if (idx > static_cast<intptr_t>(Obj8CSL::dref_values.size()) || idx < 0) {
		return;
	}
	Obj8CSL::dref_values[idx] = inValue;
}

void
Obj8CSL::Init()
{
	// set up the animation datarefs
	for (intptr_t n = 0; dref_names[n] != nullptr; n++) {
		XPLMRegisterDataAccessor(dref_names[n],
			xplmType_Float,
			true,  // writable
			nullptr, nullptr, // int
            Obj8CSL::obj8_dref_read, Obj8CSL::obj8_dref_write, // float
			nullptr, nullptr, // double
			nullptr, nullptr, // int array
			nullptr, nullptr, // float array
			nullptr, nullptr, // data
			reinterpret_cast<void *>(n), // read refcon
			reinterpret_cast<void *>(n)	 // write refcon
		);
		Obj8CSL::dref_values.emplace_back(0.0f);
	}
}

Obj8CSL::Obj8CSL(std::vector<std::string> dirNames, std::string objectName) :
	CSL(std::move(dirNames)),
	mObjectName(std::move(objectName))
{
}

string
Obj8CSL::getModelName() const
{
	string modelName = "";
	for (const auto &dir: mDirNames) {
		modelName += dir;
		modelName += ' ';
	}
	modelName += mObjectName;
	return modelName;
}

std::string
Obj8CSL::getModelType() const
{
	return "Obj8";
}

void
Obj8CSL::newInstanceData(CSLInstanceData *&newInstanceData) const
{
	newInstanceData = new Obj8InstanceData();
}

