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

#ifndef CSL_H
#define CSL_H

#include <string>
#include <vector>
#include <XPLMPlanes.h>
#include <XPMPMultiplayer.h>

#include "CullInfo.h"

// forward declare XPMPPlane - we can't access it's details, but we can record info.
class XPMPPlane;

enum class VerticalOffsetSource {
    None,
    Model,
    Mtl,
    Preference
};

class CSL;

class CSLInstanceData {
public:
    float mDistanceSqr;        // the distance squared
    bool mTCAS = false;
    bool mCulled = false;
    bool mClamped = false;

    virtual ~CSLInstanceData() = default;

    friend class CSL;

protected:
    CSLInstanceData() = default;

    /** the CSL parent class uses this method to update the individual
     * instances
     *
     * @param csl the CSL record performing the update
     * @param x X coordinate of the instance (in world units)
     * @param y Y coordinate of the instance (in world units)
     * @param z Z coordinate of the instance (in world units)
     * @param pitch
     * @param roll
     * @param heading
     * @param lights xpmp_LightStatus containing the light states for this instance
     * @param state XPLMPlaneDrawState_t containing the aircraft state for this instance
     */
    virtual void updateInstance(
        CSL *csl,
        double x,
        double y,
        double z,
        double pitch,
        double roll,
        double heading,
        xpmp_LightStatus lights,
        XPLMPlaneDrawState_t *state) = 0;
};

/** a CSL represents a single multiplayer aircraft model with livery that can be
 * rendered.
 *
 * CSL itself is the abstract - each of the renderable CSL types needs to
 * subclass and implement the relevant hooks
 */
class CSL {
public:
    /** getVertOffset returns the configured Z offset for this aircraft.
     *
     * @return Z offset in world units.
     */
    double getVertOffset() const;

    /** getVertOffsetSource() returns the identity of where the VertOffset came
     * from
     *
     * @return value from the VerticalOffsetSource enumeration identifying where
     *   the VertOffset value came from.
     */
    VerticalOffsetSource getVertOffsetSource() const;

    /** setVertOffsetSource explicit sets which vertical offset to use for
     * a given CSL.
     *
     * @param offsetSource value from the VerticalOffsetSource enumeration
     *   identifying which VertOffset should be used in preference.
     */
    void setVertOffsetSource(VerticalOffsetSource offsetSource);


    void setVerticalOffset(VerticalOffsetSource src, double offset);

    /** getModelName should return a meaningful name to reference the CSL in question
     *
     * @returns a string identifying the particular model in use
     */
    virtual std::string getModelName() const = 0;

    /** getModelType should return a short string identifying the type of CSL it is
     *
     * @returns a string identifying the type of model in use
     */
    virtual std::string getModelType() const = 0;

    /** setMovingGear is used to disable gear-position clamping.
     *
     * In order to prevent animation weirdness, if movingGear is set to false,
     * the gear position is clamped to 1 in the animation state.
     *
     * @param movingGear
     */
    void setMovingGear(bool movingGear);

    /** getMovingGear gets the state of gear-position clamping.
     */
    bool getMovingGear() const;


    /** isUsable() indicates if the CSL is suitable/available for use.
     *
     * @return returns true if the CSL is suitable for model matching, false
     *    otherwise.
     *
     * @note this largely exists in the API to permit runtime filtering of
     *          rendering methods the system is unable to support.  It made more
     *          sense before the legacy CSL renderer was removed completely.
     */
    virtual bool isUsable() const;

    void setICAO(std::string icaoCode);

    void setAirline(std::string icaoCode, std::string airline);

    void setLivery(std::string icaoCode,
                   std::string airline,
                   std::string livery);

    std::string getICAO() const;

    std::string getAirline() const;

    std::string getLivery() const;

    /** updateInstance updates the instanceData for rendering this frame.  If
     * the instanceData is not initialised, this method invokes the
     * newInstanceData virtual method to produce it.
     *
     * @param cullInfo the CullInfo to use to cull objects
     * @param x
     * @param y
     * @param z
     * @param pitch
     * @param roll
     * @param heading
     * @param lights
     * @param state
     * @param instanceData the instanceData pointer in the XPMPPlane for this plane
     */
    virtual void updateInstance(const CullInfo &cullInfo,
                                double &x,
                                double &y,
                                double &z,
                                double roll,
                                double heading,
                                double pitch,
                                bool clampToSurface,
                                float offsetScale,
                                xpmp_LightStatus lights,
                                CSLInstanceData *&instanceData,
                                XPLMPlaneDrawState_t *state);

    /* drawPlane is responsible for rendering the plane.
     */
    virtual void drawPlane(CSLInstanceData *instanceData,
                           bool is_blend,
                           int data) const;

    std::string mICAO;          // Icao type of this model
    std::string mAirline;       // Airline identifier. Can be empty.
    std::string mLivery;        // Livery identifier. Can be empty.
    bool mMovingGear;    // Does gear retract?
    VerticalOffsetSource mOffsetSource;

protected:
    CSL();

    /** Initialise the common internal structures in the CSL abstract.
     *
     * @param dirNames Relative path components (POSIX style) to the location
     *          of the xsb_aircrafts.txt file
     */
    CSL(std::vector<std::string> dirNames);

    /** newInstanceData produces a CSLInstanceData subclass to maintain the
     * state information for a CSL instance that we want to track/render.
     *
     * @param newInstanceData CSLInstanceData pointer to update.  Can be set to
     *          nullptr if we're not interested in tracking the object yet
    */
    virtual void newInstanceData(CSLInstanceData *&newInstanceData) const = 0;

    std::vector<std::string> mDirNames;       // Relative directories from X-Plane system directory to the to xsb_aircraft.txt file

    // as defined in the Model definition
    double mModelVertOffset = 0.0;
    // vert offset from mtl file
    double mMtlVertOffset = 0.0;
    // vert offset from preferences (user)
    double mPreferencesVertOffset = 0.0;
};

#endif //CSL_H
