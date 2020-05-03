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

#include <XPLMDataAccess.h>
#include <XPLMPlanes.h>
#include <XPLMUtilities.h>
#include <XPLMPlugin.h>

#include "PlanesHandoff.h"

/* forward declarations of our callbacks */
static void
wantplanes_dref_updated(void *inRefcon);

static void
planes_became_available(void *inRefcon);

static XPLMDataRef gWantPlanesDref = NULL;

static int
find_or_create_wantsplanes_dref(void)
{
    if (gWantPlanesDref != NULL) {
        return 0;
    }
    gWantPlanesDref = XPLMFindDataRef(PLANES_SAFEACQUIRE_DREF_NAME);
    if (!XPLMShareData(PLANES_SAFEACQUIRE_DREF_NAME,
                       xplmType_Int,
                       wantplanes_dref_updated,
                       NULL)) {
        XPLMDebugString(XPMP_CLIENT_NAME ":  Unable to create the reference dataref as it already exists with the wrong type");
        return 1;
    }
    if (NULL == gWantPlanesDref) {
        gWantPlanesDref = XPLMFindDataRef(PLANES_SAFEACQUIRE_DREF_NAME);
        /* set the plugin-id to no plugin */
        XPLMSetDatai(gWantPlanesDref, XPLM_NO_PLUGIN_ID);
    }
    return 0;
}

static StateCallback_f gAcquireCallback = NULL;
static StateCallback_f gReleaseCallback = NULL;
static void * gStateRefcon = NULL;

/* gWantToAcquire is used to indicate if we have an outstanding acquisition
 * callback in flight.  This is important as it changes our behaviour around
 * a few certain things.
*/
static int gWantToAcquire = 0;

static void
finalise_planes_acquisition()
{
    if (gAcquireCallback != NULL) {
        gAcquireCallback(gStateRefcon);
    }
    gWantToAcquire = 0;
}

int
Planes_SafeAcquire(StateCallback_f acquirePtr,
                   StateCallback_f releasePtr,
                   void *refcon,
                   unsigned int flags)
{
    if (find_or_create_wantsplanes_dref()) {
        return PLANES_SAFEACQUIRE_ERROR;
    }
    /* already have an outstanding acquisition */
    if (gWantToAcquire) {
        return PLANES_SAFEACQUIRE_ERROR;
    }

    /* check to see if the existing consumer is passive if we don't want to
     * steal from active clients.
    */
    if (flags & PLANES_SAFEACQUIRE_TAKE_ONLY_FROM_PASSIVE) {
        XPLMPluginID currentOwner = XPLMGetDatai(gWantPlanesDref);
        if (currentOwner != XPLM_NO_PLUGIN_ID) {
            return PLANES_SAFEACQUIRE_ERROR;
        }
    }

    /* attempt to claim the plugins and ask other consumers to relinquish
     * their control of the planes
    */
    if (!(flags & PLANES_SAFEACQUIRE_PASSIVE_ONLY)) {
        XPLMSetDatai(gWantPlanesDref, XPLMGetMyID());
    }
    gWantToAcquire = 1;
    gAcquireCallback = acquirePtr;
    gReleaseCallback = releasePtr;
    gStateRefcon = refcon;
    int result = XPLMAcquirePlanes(NULL, (flags & PLANES_SAFEACQUIRE_NOWAIT)?NULL:planes_became_available, NULL);
    if (result) {
        finalise_planes_acquisition();
        return PLANES_SAFEACQUIRE_OK;
    }
    return (flags & PLANES_SAFEACQUIRE_NOWAIT)?PLANES_SAFEACQUIRE_ERROR:PLANES_SAFEACQUIRE_OK;
}

static int
planes_amIActive()
{
    XPLMPluginID controllingPlugin = XPLM_NO_PLUGIN_ID;
    XPLMCountAircraft(NULL, NULL, &controllingPlugin);
    return (controllingPlugin == XPLMGetMyID());
}

void
Planes_SafeRelease(void)
{
    if (gWantToAcquire) {
        gWantToAcquire = 0;
    } else {
        /* irrespective as to if we're actually on the mutex or not, we need
         * to let the client know to clean up.
        */
        if (gReleaseCallback != NULL) {
            gReleaseCallback(gStateRefcon);
        }
        if (planes_amIActive()) {
            XPLMReleasePlanes();
        }
    }
    /* cleanup the callbacks */
    gAcquireCallback = NULL;
    gReleaseCallback = NULL;
    gStateRefcon = NULL;

    /* we flip the dataref *after* we do the release to try to dodge double-steps
     * where we notify that we're relinquishing control, and having another
     * consumer step up and have to spin on the mutex still being held.
     *
     * If the wantedInControl value still suggests it's us, we flip it to
     * nobody.
    */
    XPLMPluginID wantedInControl = (XPLMPluginID)XPLMGetDatai(gWantPlanesDref);
    if (wantedInControl == XPLMGetMyID()) {
        XPLMSetDatai(gWantPlanesDref, XPLM_NO_PLUGIN_ID);
    }
}

static void
wantplanes_dref_updated(void *inRefcon)
{
    XPLMPluginID desiredPlugin = (XPLMPluginID)XPLMGetDatai(gWantPlanesDref);
    if (planes_amIActive()) {
        /* if we are in control, and the ID changes to somebody else, we need
         * to let go of our control
        */
        if (desiredPlugin != XPLMGetMyID() &&
            desiredPlugin != XPLM_NO_PLUGIN_ID) {
            Planes_SafeRelease();
            return;
        }
    } else if (gWantToAcquire) {
        if (desiredPlugin == XPLM_NO_PLUGIN_ID) {

        }

    }

};

static void
planes_became_available(void *inRefcon)
{
    XPLMPluginID desiredPlugin = (XPLMPluginID)XPLMGetDatai(gWantPlanesDref);
    if (gWantToAcquire && (desiredPlugin == XPLM_NO_PLUGIN_ID || desiredPlugin == XPLMGetMyID())) {
        int result = XPLMAcquirePlanes(NULL, planes_became_available, NULL);
        if (result) {
            finalise_planes_acquisition();
        }
    }
}

int
Planes_AcquisitionStatus(XPLMPluginID *outControllingPlugin)
{
    if (find_or_create_wantsplanes_dref()) {
        return PLANES_SAFEACQUIRE_ERROR;
    }
    XPLMPluginID desiredPlugin = (XPLMPluginID)XPLMGetDatai(gWantPlanesDref);
    XPLMPluginID controllingPlugin = XPLM_NO_PLUGIN_ID;
    XPLMCountAircraft(NULL, NULL, &controllingPlugin);
    if (outControllingPlugin != NULL) {
        *outControllingPlugin = controllingPlugin;
    }
    /* first.. deal with mismatches between consumer and claimant */
    if (desiredPlugin != controllingPlugin) {
        /* if they're not claiming to be the consumer, then they're probably
         * passive
        */
        if (desiredPlugin == XPLM_NO_PLUGIN_ID) {
            return PLANES_SAFEACQUIRE_STATE_PASSIVE_CONSUMER;
        }
        return PLANES_SAFEACQUIRE_STATE_LEGACY_CONSUMER;
    }
    if (controllingPlugin == XPLM_NO_PLUGIN_ID) {
        return PLANES_SAFEACQUIRE_STATE_UNCLAIMED;
    }
    return PLANES_SAFEACQUIRE_STATE_ACTIVE_CONSUMER;
}
