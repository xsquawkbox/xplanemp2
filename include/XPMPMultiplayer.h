/*
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
 * Copyright (c) 2018,2020, Chris Collins.
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

#ifndef _XPLMMultiplayer_h_
#define _XPLMMultiplayer_h_

#include <stddef.h>

#include "XPLMDefs.h"

#ifndef XPMP_CLIENT_NAME
#define XPMP_CLIENT_NAME "A CLIENT"
#endif

#ifndef XPMP_CLIENT_LONGNAME
#define XPMP_CLIENT_LONGNAME "A Client"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************
 * X-PLANE MULTIPLAYER
 ************************************************************************************/

/**
 * @mainpage
 *
 * X-Plane Multiplayer Library 2.x (for X-Plane 11 and newer).
 *
 * a.k.a:  `xplanemp` or `xpmp2`
 *
 * Historically, X-Plane only has a very limited integrated traffic system - it
 * uses full detail ACFs and simulates the flight model for each aircraft every
 * frame.    This is fine for light simulated traffic, but wasn't enough for
 * high traffic environments (such as VATSIM or IVAO) or lower performance
 * systems.
 *
 * xplanemp was originally glue that provided OpenGL-based independent traffic
 * rendering inside the simulator, allowing users to provide simplified 3D
 * models which the pilot client could have injected into the world, without
 * incurring the full overhead of an ACF.
 *
 * X-Plane has changed - now it has multiple rendering backends and we have a
 * API for loading and rendering instances of standard objects.  xplanemp
 * provides the model definition and matching glue, efficient resource
 * management, and participation in a few key hacks (such as TCAS faking, ground
 * level clamping, amongst others).
 *
 * Because we can no longer trivially tell if an aircraft's data is needed as
 * we no longer directly control rendering, the API has been shifted to a push
 * model for data, where clients push updates for registered aircraft every
 * frame.
 *
 * Collections of aircraft models are known as CSLs (short for "Common Shape
 * Library") as that was the term used by the Microsoft Flight Simulator client,
 * SquawkBox, before XSquawkBox, the first major X-Plane client and the origin
 * for a good portion of the original xplanemp code, came into existence.
 *
 * `xpmp2` is **NOT** API compatible with the original `xplanemp`.  Please
 * study the source and this documentation carefully before including it into
 * an existing project.
*/

/** XPMPConfiguration_t contains all of the configurable paramaters for
 * libxplanemp
 *
 * This is not size-keyed as libxplanemp /should/ be directly linked to it's
 * main consumer, and so there shouldn't be any way for this to be out of step
 * with it's actual use.
 */
typedef struct XPMPConfiguration_s {
	float					maxFullAircraftRenderingDistance;	/// Beyond what distance do we start using lights-only rendering?
	bool 					enableSurfaceClamping;		/// do we clamp all aircraft to the surface?
	struct {
		bool modelMatching;								/// Enable Verbose Debugging about Model matching
		bool allowObj8AsyncLoad;						/// Enable the asynchronous Obj8 model loader (was buggy)
	} debug;
} XPMPConfiguration_t;


/**
 * XPMPSetConfiguration sets the new configuration parameters in libxplanemp
 *
 * @param newConfig new set of configuration paramters to apply
 */
void XPMPSetConfiguration(XPMPConfiguration_t *inConfig);

/**
 * XPMPGetConfiguration gets the current configuration parameters from
 * libxplanemp
 *
 * @param outConfig location to write the configuration parameters to.
 */
void XPMPGetConfiguration(XPMPConfiguration_t *outConfig);



/* ***********************************************************************************
 *  PLANE DATA TYPES
 * ***********************************************************************************/

/**
 * XPMPPosition_t contains the basic position info for an aircraft.size-size
 *
 * Lat and lon are the position of the aircraft in the world.  They are double-precision to
 * provide reasonably precise positioning anywhere.  Elevation is in feet above mean sea level.
 *
 * Pitch, roll, and heading define the aircraft's orientation.  Heading is in degrees, positive
 * is clockwise from north.  Pitch is the number of degrees, positive is nose up, and roll
 * is positive equals roll right.
 *
 * Note that there is no notion of aircraft velocity or acceleration; you will be queried for
 * your position every rendering frame.  Higher level APIs can use velocity and acceleration.
 *
 */
typedef	struct {
	size_t	size;
	double	lat;
	double	lon;
	double	elevation;
	float	pitch;
	float	roll;
	float	heading;
	char 	label[32];
    float 	offsetScale;
    bool 	clampToGround;
} XPMPPlanePosition_t;


/** The XPMPLightStatus enum defines the settings for the lights bitfield in XPMPPlaneSurfaces_t
 *
 * The upper 16 bit of the light code (timeOffset) should be initialized only once
 * with a random number by the application. This number will be used to have strobes
 * flashing at different times.
 */
union xpmp_LightStatus {
	unsigned int lightFlags;
	struct {
		unsigned int timeOffset	: 16;

		unsigned int taxiLights : 1;
		unsigned int landLights	: 1;
		unsigned int bcnLights	: 1;
		unsigned int strbLights	: 1;
		unsigned int navLights	: 1;
		
		unsigned int flashPattern   : 4;
	};
};

/**
 * Light flash patterns
 */
enum {
	xpmp_Lights_Pattern_Default		= 0,	// Jets: one strobe flash, short beacon (-*---*---*---)
	xpmp_Lights_Pattern_EADS		= 1,	// Airbus+EADS: strobe flashes twice (-*-*-----*-*--), short beacon
	xpmp_Lights_Pattern_GA			= 2		// GA: one strobe flash, long beacon (-*--------*---)
};


/**
 * XPMPPlaneSurfaces_t will contain information about the external physical configuration of the plane,
 * things you would notice if you are seeing it from outside.  This includes flap position, gear position,
 * etc.
 *
 * Lights is a 32 bit field with flags as defined in XPMPLightStatus
 *
 */
typedef	struct {
	size_t				  size;
	float                 gearPosition;
	float                 flapRatio;
	float                 spoilerRatio;
	float                 speedBrakeRatio;
	float                 slatRatio;
	float                 wingSweep;
	float                 thrust;
	float                 yokePitch;
	float                 yokeHeading;
	float                 yokeRoll;
	xpmp_LightStatus      lights;
} XPMPPlaneSurfaces_t;


/**
 * XPMPTransponderMode
 *
 * These enumerations define the way the transponder of a given plane is operating.
 *
 */
enum {
	xpmpTransponderMode_Standby,
	xpmpTransponderMode_Mode3A,
	xpmpTransponderMode_ModeC,
	xpmpTransponderMode_ModeC_Low,
	xpmpTransponderMode_ModeC_Ident
};
typedef	int	XPMPTransponderMode;

/**
 * XPMPPlaneSurveillance_t defines information about an aircraft visible to SSR.
 */
typedef	struct {
	size_t					size;
	int 					code;
	XPMPTransponderMode		mode;
} XPMPPlaneSurveillance_t;

/**
 * XPMPPlaneID is a unique ID for an aircraft created by a plug-in.
 *
 */
typedef	void *		XPMPPlaneID;

/** XPMPPlaneUpdate is used to feed updates in aircraft state data into libxplanemp
 */
typedef struct {
    /// plane refers to the plane to update - it can be set to 0, in which case
    /// that update will be ignored in it's entirety.
	XPMPPlaneID				plane;
	XPMPPlanePosition_t		*position;
	XPMPPlaneSurfaces_t		*surfaces;
	XPMPPlaneSurveillance_t *surveillance;
} XPMPUpdate_t;

/************************************************************************************
* Some additional functional by den_rain
************************************************************************************/

void actualVertOffsetInfo(const char *inMtl, char *outType, double *outOffset);
void setUserVertOffset(const char *inMtlCode, double inOffset);
void removeUserVertOffset(const char *inMtlCode);


/************************************************************************************
 * PLANE CREATION API
 ************************************************************************************/

/** XPMPMultiplayerInit sets up the libxplanemp library using the provided paths.
 *
 * @param inConfiguration can point to a XPMPConfiguration_t with the initial parameters for the library
 * @param inRelated path to the related.txt table
 * @param inDoc8643 path to the doc8643.txt table
 * @return NULL if OK, a C string if an error occured.
 */
const char *
XPMPMultiplayerInit(XPMPConfiguration_t *inConfiguration,
                    const char *inRelated,
                    const char *inDoc8643);

/** XPMPMultiplayerLoadCSLPackages loads all CSL packages from the nominated path
 *
 * @param inPackagePath relative (or absolute) path to the base folder for a CSL package
 * @return NULL if the load was successful, otherwise a C string with an error message describing the failure
 *
 * @note The error message is actually generic - it just refers the user to read
 *    the log file.
 */
const char *	XPMPMultiplayerLoadCSLPackages(const char * inPackagePath);

/*
 * XPMPMultiplayerEnable
 *
 * Enable drawing of multiplayer planes.  Call this once from your XPluginEnable routine to
 * grab multiplayer; an empty string is returned on success, or a human-readable error message
 * otherwise.
 *
 */
const char *	XPMPMultiplayerEnable(void);



/*
 * XPMPMultiplayerDisable
 *
 * Disable drawing of multiplayer planes.  Call this from XPluginDisable to release multiplayer.
 * Reverses the actions on XPMPMultiplayerEnable.
 */
void XPMPMultiplayerDisable(void);

/*
 * XPMPMultiplayerCleanup
 *
 * Clean up the multiplayer library. Call this from XPluginStop to reverse the actions of
 * XPMPMultiplayerInit as much as possible.
 */
void XPMPMultiplayerCleanup(void);

/** XPMPLoadCSLPackages loads a collection of packages (which in turn contain
 * collections of planes)
 *
 * This is fast as all plane object files are loaded asynchronously on demand,
 * but it's still probably a good idea not to invoke this whilst you're
 * performance critical..
 *
 * @param inCSLFolder path to the parent folder to scan for packages.
 * @return NULL if OK, a C string if an error occured.
 */
const char *	XPMPLoadCSLPackages(const char * inCSLFolder);

/** XPMPGetNumberOfInstalledModels returns the number of loaded models.
 *
 * @returns total count of all plane models currently registered.
 */
int XPMPGetNumberOfInstalledModels(void);

/** XPMPGetModelInfo returns information about the model referred to by index
 * inIndex.
 *
 * Valid index values are between 0 and XPMPGetNumberOfInstalledModels()
 * inclusive. If you pass an index out of this range, the out parameters are
 * unchanged.
 *
 * return values must not be modified in place and are not guaranteed to persist
 * if the plugin is disabled.
 *
 * @note the load order of models is highly-likely non-deterministic, and may
 * vary from start to start.
 *
 * @param inIndex index of the aircraft model to get data on
 * @param outModelName pointer to a const char pointer to be pointed to the model name.
 * @param outIcao pointer to a const char pointer to be pointed to the ICAO type
 * @param outAirline pointer to a const char pointer to be pointed to the Airline
 * @param outLivery pointer to a const char pointer to be pointed to the Livery
 */
void XPMPGetModelInfo(int inIndex, const char **outModelName, const char **outIcao, const char **outAirline, const char **outLivery);

/** XPMPCreatePlane creates a new plane for a plug-in and returns its ID.
 * The new aircraft will have a model and livery assigned based on the
 * ICAO/Airline/Livery triplet.
 *
 * Pass in an ICAO aircraft ID code, airline and a livery string.
 *
 * Undetermined ICAOCodes should be specified as "????".
 *
 * Undetermined Livery or Airline codes should be specified as the empty string.
 *
 * @param inICAOCode ICAO code for the new aircraft
 * @param inAirline Airline code for the new aircraft
 * @param inLivery Livery code for the new aircraft
 * @return an opaque ID for the plane
 */
XPMPPlaneID	XPMPCreatePlane(
		const char *			inICAOCode,
		const char *			inAirline,
		const char *			inLivery);

/** XPMPCreatePlane creates a new plane for a plug-in and returns its ID.
 * The new aircraft will have a model and livery assigned based on the
 * model name provided.  If that model name cannot be found, the
 * ICAO/Airline/Livery triplet is used instead.
 *
 * Undetermined ICAOCodes should be specified as "????".
 * Undetermined Livery or Airline codes should be specified as the empty string.
 *
 * @note unlike the IVAO/etc branches, ModelNames in libxplanemp official are
 *			case sensitive - this is as many POSIX systems are case sensitive,
 *			so you can have differing models that vary only by case, but also
 *			because the case of the modelname is provided in consistent form
 *			in the xsb_aircraft.txt file in every package.  Case insensitivity
 *			also makes the search even slower.
 *
 * @param inICAOCode ICAO code for the new aircraft
 * @param inAirline Airline code for the new aircraft
 * @param inLivery Livery code for the new aircraft
 * @return an opaque ID for the plane
 */
XPMPPlaneID	XPMPCreatePlaneWithModelName(
		const char *			inModelName,
		const char *			inICAOCode,
		const char *			inAirline,
		const char *			inLivery);

/** XPMPDestroyPlane deallocates a created aircraft.
 *
 * @param inID the plane to destroy
 */
void			XPMPDestroyPlane(XPMPPlaneID inID);

/** XPMPChangePlaneModel changes the active model for a plane.
 *
 * @note the Match quality is an integer - lower numbers (greater than 0) are
 * 		better, 2 or lower indicates an exact match on model.   Negative values
 * 		indicate failure to match at all.
 *
 * @param inPlaneID the plane to change the model on
 * @param inICAOCode the ICAO code of the new model
 * @param inAirline the Airline code of the new model
 * @param inLivery the Livery code of the new model
 * @param force_change if this is true, the model will be changed irrespective
 * 		of quality, otherwise changes that decrease the quality of the match
 * 		will be elided.
 * @return the match quality (see notes)
 */
int 	XPMPChangePlaneModel(
		XPMPPlaneID				inPlaneID,
		const char *			inICAOCode,
		const char *			inAirline,
		const char *			inLivery,
		int						force_change);

/** XPMPSetDefaultPlaneICAO sets the type code to be used as the fallback model
 * if all of the attempts to find a matching model fail.
 *
 * @param inICAO the ICAO code of the model to use
 */
void	XPMPSetDefaultPlaneICAO(
		const char *			inICAO);

/** XPMPCountPlanes returns the number of planes in existence.
 *
 * @return total count of planes active in libxplanemp
 */
long			XPMPCountPlanes(void);

/** XPMPGetNthPlane returns the plane ID of the Nth registered plane.
 *
 * @param index the index of the plane ID to retrieve
 * @returns the ID for the plane requested.  NULL if not found.
 */
XPMPPlaneID	XPMPGetNthPlane(
		long 					index);


/** XPMPUpdatePlanes performs a bulk update on a number of aircraft positions or
 * states
 *
 * @param inUpdates a pointer to the first element of an array of XPMPUpdate_t
 * @param inUpdateSize the size of a single XPMPUpdate_t structure
 * @param inCount the total count of elements to process.
 */
void		XPMPUpdatePlanes(
	XPMPUpdate_t *				inUpdates,
	size_t						inUpdateSize,
	size_t						inCount);

/** XPMPIsICAOValid searches the models loaded to see if
 *
 * This functions searches through our global vector of valid ICAO codes and returns true if there
 * was a match and false if there wasn't.
 *
 * @return true if a direct match, or a fallback match (but not the default
 * 			fallback) was successful.
 */
bool			XPMPIsICAOValid(
		const char *				inICAO);

/** XPMPGetPlaneModelQuality returns the quality level for the nominated plane's
 * current model.
 *
 * @return the Model quality of the current assigned model (see
 *     XPMPChangePlaneModel for more details)
 */
int 		XPMPGetPlaneModelQuality(
		XPMPPlaneID 				inPlane);

/** XPMPModelMatchQuality performs a search for the nominated type triplet
 * and returns the model quality of the result.
 *
 * This functions searches through our model list and returns the pass
 * upon which a match was found, and -1 if one was not.
 *
 * This can be used for assessing if it's worth using a partial update
 * to update the model vs previous efforts.
 *
 * @param inICAO the ICAO code of the model to search for
 * @param inAirline the Airline code of the model to search for
 * @param inLivery the Livery code of the model to search for
 * @return the model quality of the model found, or -1 if no match was found.
 */
int			XPMPModelMatchQuality(
		const char *				inICAO,
		const char *				inAirline,
		const char *				inLivery);

/************************************************************************************
 * PLANE RENDERING API
 ************************************************************************************/

/** XPMPDumpOneCycle will cause the plane renderer implementation to dump debug
 * info to the log.txt for one cycle after it is called - useful for figuring
 * out why your models don't look right.
 */
void		XPMPDumpOneCycle(void);

#ifdef __cplusplus
}
#endif


#endif
