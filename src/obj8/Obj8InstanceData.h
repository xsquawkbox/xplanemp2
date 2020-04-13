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

#ifndef OBJ8INSTANCEDATA_H
#define OBJ8INSTANCEDATA_H

#include <deque>
#include <XPLMInstance.h>

#include "Obj8Attachment.h"
#include "CSL.h"

/** a single renderable instance of a Obj8CSL */
class Obj8InstanceData : public CSLInstanceData {
public:
    const void *  mInstanceSetPtrs[Obj8DrawTypeCount];
    std::vector<XPLMInstanceRef> mInstances[Obj8DrawTypeCount];

    //std::deque<std::pair<Obj8Attachment*,XPLMInstanceRef>>     mInstances;

	Obj8InstanceData():
	    mInstanceSetPtrs{nullptr,},
	    mInstances{}
    {};

	virtual ~Obj8InstanceData() {
	    resetModel();
	};

	friend class Obj8CSL;

protected:
	void updateInstance(
		CSL *csl,
		double x,
		double y,
		double z,
		double pitch,
		double roll,
		double heading,
		xpmp_LightStatus lights,
		XPLMPlaneDrawState_t *state) override;

	void resetPartsForType(const Obj8CSL *csl, Obj8DrawType drawType);
	void instancePartsForType(const Obj8CSL *csl, Obj8DrawType drawType);

private:
	void resetModel();
};

#endif //OBJ8INSTANCEDATA_H
