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

#ifndef OBJ8ATTACHMENT_H
#define OBJ8ATTACHMENT_H

#include "Obj8Common.h"
#include <string>
#include <utility>
#include <queue>
#include <memory>
#include <unordered_map>
#include <XPLMScenery.h>

/** Obj8Attachment is a single obj8 component loaded and ready for rendering.
 */
class Obj8Attachment {
public:
    /** use this to construct Obj8Attachments - it'll handle deduplication if
     * necessary.
     *
     * @param filename POSIX path to the obj8 to load
     * @return a std::shared_ptr for the requested obj8 attachment
     */
    static std::shared_ptr<Obj8Attachment> getAttachmentForFile(const std::string &filename);

	Obj8Attachment(const Obj8Attachment &copySrc) = delete;

	Obj8Attachment(Obj8Attachment &&moveSrc) noexcept:
            mFile(std::move(moveSrc.mFile)),
            mHandle(nullptr),
            mLoadState(Obj8LoadState::None)
    {
        mHandle = moveSrc.mHandle;
        moveSrc.mHandle = nullptr;

        mLoadState = moveSrc.mLoadState;
        moveSrc.mLoadState = Obj8LoadState::None;
    }

	virtual ~Obj8Attachment();

	/** try to get the object handle.  Queue it for loading if it's not available.
	 * @returns The XPLMObjectRef for this attachment
	 */
	XPLMObjectRef	getObjectHandle() {
        switch (mLoadState) {
            case Obj8LoadState::None:
                enqueueLoad();
                return nullptr;
            case Obj8LoadState::Loaded:
                return mHandle;
            case Obj8LoadState::Failed:
            case Obj8LoadState::Loading:
                return nullptr;
        }
        return nullptr;
    }

	Obj8LoadState       getLoadState() const {
	    return mLoadState;
	}

protected:
	std::string			mFile;
	XPLMObjectRef		mHandle;
	Obj8LoadState		mLoadState;

    explicit Obj8Attachment(std::string fileName):
        mFile(std::move(fileName)),
        mHandle(nullptr),
        mLoadState(Obj8LoadState::None)
    {
    }


private:
    static std::unordered_map<std::string,std::weak_ptr<Obj8Attachment>> sAttachmentCache;
    static void	loadCallback(XPLMObjectRef inObject, void *inRefcon);
    static std::queue<Obj8Attachment *>	loadQueue;
    void enqueueLoad();
};

#endif //OBJ8ATTACHMENT_H
