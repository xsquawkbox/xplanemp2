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

#include "Obj8Attachment.h"

#include <queue>
#include <XPLMScenery.h>
#include <XUtils.h>

std::queue<Obj8Attachment *>	Obj8Attachment::loadQueue;
std::unordered_map<std::string,std::weak_ptr<Obj8Attachment>> Obj8Attachment::sAttachmentCache;

void
Obj8Attachment::loadCallback(XPLMObjectRef inObject, void *inRefcon)
{
    auto *sThis = reinterpret_cast<Obj8Attachment *>(inRefcon);

    sThis->mHandle = inObject;
    if (nullptr == inObject) {
        sThis->mLoadState = Obj8LoadState::Failed;
        XPLMDump() << XPMP_CLIENT_NAME << " failed to load obj8: " << sThis->mFile << "\n";
    } else {
        XPLMDump() << XPMP_CLIENT_NAME << " did load obj8: " << sThis->mFile << "\n";
        sThis->mLoadState = Obj8LoadState::Loaded;
    }

    if (!loadQueue.empty()) {
        Obj8Attachment *nextAtt = nullptr;
        nextAtt = loadQueue.front();
        loadQueue.pop();
        if (nextAtt) {
            XPLMLoadObjectAsync(nextAtt->mFile.c_str(), &Obj8Attachment::loadCallback, reinterpret_cast<void *>(nextAtt));
        }
    }
}

std::shared_ptr<Obj8Attachment>
Obj8Attachment::getAttachmentForFile(const std::string &filename)
{
    auto wpIter = sAttachmentCache.find(filename);
    if (wpIter != sAttachmentCache.end()) {
        auto sp = wpIter->second.lock();
        if (sp) {
            return std::move(sp);
        }
    }
    auto sp = std::shared_ptr<Obj8Attachment>(new Obj8Attachment(filename));
    sAttachmentCache[filename] = sp;
    return std::move(sp);
}

void
Obj8Attachment::enqueueLoad() {
    if (mLoadState != Obj8LoadState::None) {
        return;
    }
    if (mFile.empty()) {
        return;
    }
    mLoadState = Obj8LoadState::Loading;
    if (loadQueue.empty()) {
        XPLMLoadObjectAsync(mFile.c_str(), &Obj8Attachment::loadCallback, reinterpret_cast<void *>(this));
    } else {
        loadQueue.push(this);
    }
};


Obj8Attachment::~Obj8Attachment()
{
    if (mHandle != nullptr) {
        XPLMUnloadObject(mHandle);
        mLoadState = Obj8LoadState::None;
    }
}
