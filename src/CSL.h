//
// Created by kuroneko on 2/03/2018.
//

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
class CSLInstanceData
{
protected:
	CSLInstanceData() = default;

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

public:
	float	mDistanceSqr;		// the distance squared
	bool	mTCAS = false;
	bool	mCulled = false;
	bool	mClamped = false;


	virtual ~CSLInstanceData() = default;

	friend class CSL;
};

class RenderedCSLInstanceData : public CSLInstanceData
{
public:
	RenderedCSLInstanceData() = default;

	double					mX = 0.0;
	double					mY = 0.0;
	double					mZ = 0.0;
	double				 	mPitch = 0.0;
	double				 	mRoll = 0.0;
	double				 	mHeading = 0.0;
	int					 	mFull = false;
	xpmp_LightStatus	 	mLights;
	XPLMPlaneDrawState_t 	mState;
protected:

	virtual void updateInstance(
		CSL *csl,
		double x,
		double y,
		double z,
		double pitch,
		double roll,
		double heading,
		xpmp_LightStatus lights,
		XPLMPlaneDrawState_t *state);

	friend class CSL;
};

/** a CSL is a single multiplayer aircraft that can be rendered.
 *
 * CSL itself is the abstract - each of the aircraft types needs to subclass and implement the relevant hooks
 */
class CSL
{
public:
	double getVertOffset() const;

	VerticalOffsetSource getVertOffsetSource() const;

	void setVertOffsetSource(VerticalOffsetSource offsetSource);

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

	void setVerticalOffset(VerticalOffsetSource src, double offset);

	/** setMovingGear is used to disable gear-position clamping.
	 *
	 * In order to prevent animation weirdness, if movingGear is set to false,
	 * the gear position is clamped to 1 in the animation state.
	 *
	 * @param movingGear
	 */
	void setMovingGear(bool movingGear);

	bool getMovingGear() const;


	/** isUsable() indicates if the CSL is suitable/available for use.
	 *
	 * @return returns true if the CSL is suitable for model matching, false otherwise
	 */
	virtual bool	isUsable() const;

	void setICAO(std::string icaoCode);
	void setAirline(std::string icaoCode, std::string airline);
	void setLivery(std::string icaoCode, std::string airline, std::string livery);

	std::string		getICAO() const;
	std::string 	getAirline() const;
	std::string		getLivery() const;

	virtual void newInstanceData(CSLInstanceData *&newInstanceData) const;

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
	virtual void updateInstance(
		const CullInfo &cullInfo,
		double &x,
		double &y,
		double &z,
		double roll,
		double heading,
		double pitch,
		bool clampToSurface,
		xpmp_LightStatus lights,
		CSLInstanceData *&instanceData,
		XPLMPlaneDrawState_t *state);

	/* drawPlane is responsible for rendering the plane.
	 */
 	virtual void drawPlane(CSLInstanceData *instanceData, bool is_blend, int data) const;

	std::string					mICAO;          // Icao type of this model
	std::string 				mAirline;       // Airline identifier. Can be empty.
	std::string 				mLivery;        // Livery identifier. Can be empty.
	bool 						mMovingGear;	// Does gear retract?
	VerticalOffsetSource		mOffsetSource;
protected:
	CSL();
	CSL(std::vector<std::string> dirNames);

	std::vector<std::string>	mDirNames;       // Relative directories from xsb_aircrafts.txt down to object file


	// as defined in the Model definition
	double mModelVertOffset = 0.0;
	// vert offset from mtl file
	double mMtlVertOffset = 0.0;
	// vert offset from preferences (user)
	double mPreferencesVertOffset = 0.0;
};

#endif //CSL_H
