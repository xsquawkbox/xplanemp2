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

#ifndef OBJ8CSL_H
#define OBJ8CSL_H

#include <unordered_map>
#include <queue>

#include <XPLMScenery.h>
#include <XPLMInstance.h>

#include "CullInfo.h"
#include "CSL.h"
#include "Obj8Common.h"
#include "Obj8Attachment.h"

class Obj8CSL: public CSL {
public:
    using attachment_pointer = std::shared_ptr<Obj8Attachment>;
    using attachment_array = std::vector<attachment_pointer>;
    using attachment_map = std::unordered_map<Obj8DrawType, attachment_array>;

    void newInstanceData(CSLInstanceData *&newInstanceData) const override;

    Obj8CSL(std::vector<std::string> dirNames, std::string objectName);

    void addAttachment(Obj8DrawType draw_type, attachment_pointer att)
    {
        mAttachments[draw_type].emplace_back(std::move(att));
    }

    bool hasAttachmentsFor(Obj8DrawType drawType) const
    {
        auto attIter = mAttachments.find(drawType);
        if (attIter == mAttachments.end()) {
            return false;
        }
        return !(attIter->second.empty());
    };

    const attachment_array *
    getAttachmentsFor(Obj8DrawType drawType) const
    {
        auto attIter = mAttachments.find(drawType);
        if (attIter == mAttachments.end()) {
            return nullptr;
        }
        return &(attIter->second);
    }

    std::string getModelName() const override;

    std::string getModelType() const override;

    static void Init();
    static const char * dref_names[];
protected:

    attachment_map mAttachments;
    std::string mObjectName;     // Basename of the object file

private:

    /* these  statics are used for passing animation datarefs into non-instanced
     * rendering
    */
    static float    obj8_dref_read(void *inRefcon);
    static void     obj8_dref_write(void *inRefcon, float inValue);
    static std::vector<float> dref_values; // from Obj8CSL.cpp
};


#endif //OBJ8CSL_H
