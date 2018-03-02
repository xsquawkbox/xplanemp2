//
// Created by kuroneko on 2/03/2018.
//

#ifndef CSL_H
#define CSL_H

#include <string>
#include <vector>
#include <XPLMPlanes.h>
#include <XPMPMultiplayer.h>

enum class CSLType {
	Austin,
	Obj,
	Lights,
	Obj8,
	Count
};

enum class VerticalOffsetSource {
	None,
	Model,
	Mtl,
	Preference
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

	/** needRenderCallback is used to validate if the CSL requires hooks in the rendering loop to be drawn.
	 *
	 * @returns true if a rendering hook is required, false otherwise.
	 */
	virtual bool needsRenderCallback() = 0;

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

	/* drawPlane renders the plane when called from within the rendering callback */
	virtual void drawPlane(
		float distance,
		double x,
		double y,
		double z,
		double pitch,
		double roll,
		double heading,
		int full,
		xpmp_LightStatus lights,
		XPLMPlaneDrawState_t *state,
		void *&instanceData) = 0;

	/* newInstanceData attaches state data to the plane object, if necessary.
	 *
	 * @returns an opaque datareference for the renderer to use to track state if necessary, nullptr otherwise
	 */
	virtual void *		newInstanceData();

	/* deleteInstanceData releases instance data previously generated using newInstanceData()
	 *
	 * @param instanceData a result from a previous call of newInstanceData().
	 */
	virtual void		deleteInstanceData(void *instanceData);
protected:
	CSL();
	CSL(std::vector<std::string> dirNames);

	std::string mICAO;           // Icao type of this model
	std::string mAirline;        // Airline identifier. Can be empty.
	std::string mLivery;         // Livery identifier. Can be empty.

	std::vector<std::string>	mDirNames;       // Relative directories from xsb_aircrafts.txt down to object file

	bool 						mMovingGear;	// Does gear retract?

	VerticalOffsetSource		mOffsetSource;

	// as defined in the Model definition
	double mModelVertOffset = 0.0;
	// vert offset from mtl file
	double mMtlVertOffset = 0.0;
	// vert offset from preferences (user)
	double mPreferencesVertOffset = 0.0;
};


#if 0
	std::string 				mObjectName;     // Basename of the object file
	std::string 				mTextureName;    // Basename of the texture file


	std::string mFilePath;        // Where do we load from (oz and obj, debug-use-only for OBJ8)

	/* distance between the origin point of an object and the lowest point of the object
	 * (usually a bottom point of the gears) along the vertical axis (y axis in the sim)
	 * for onground clamping purposes.
	 *
	 * in simple words, correct vert offset for accurate putting planes on the ground.
	 * (in meters)
	 */

};

class AustinCSL : public CSL {
protected:
	// plane_Austin
	int austin_idx;

};

class Obj8CSL : public CSL {
protected:
	// plane_Obj8
	vector <obj_for_acf> attachments;
};
#endif

#endif CSL_H
