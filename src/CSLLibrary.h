/*
 * Copyright (c) 2005, Ben Supnik and Chris Serio.
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

#ifndef XPLMMULTIPLAYERCSL_H
#define XPLMMULTIPLAYERCSL_H

/*
 * XPLMMultiplayerCSL
 *
 * This unit is the master switch for managing aircraft models.  It loads up all CSL packages and
 * does the matching logic for finding aircraft.
 *
 */

#include <XPLMPlanes.h>
#include "XPMPMultiplayerVars.h"

/*
 * CSL_Init
 *
 * This routine Initializes the Multiplayer object section and sets up the lighting texture.
 *
 */
bool			CSL_Init(
		const char* inTexturePath);
/*
 * CSL_LoadCSL
 *
 * This routine loads all CSL packages and the related.txt file.
 *
 */
bool			CSL_LoadCSL(
		const char * inFolderPath); 		// Path to CSL folder

/** CSL_LoadData starts loading the non-package data required to handle the CSL
 * library.
 *
 * If there are any issues, details about the cause are sent to the XPlane log.
 *
 * @param inRelated path to the related.txt - used by the renderer to pick a suitable model when no specific model is available
 * @param inDoc8643 path to the doc8643.txt table - used by the renderer to pick a suitable model when there's no specific model and the related types don't help
 *
 * @returns true if successful, false if there were any issues.
 */
bool			CSL_LoadData(
	const char * inRelated,			// Path to related.txt - used by renderer for model matching
	const char * inDoc8643);		// Path to ICAO document 8643 (list of aircraft)

/** CSL_LoadCSL loads all of the packages underneath the specified path
 *
 * If there are any issues, details about the cause are sent to the XPlane log.
 *
 * @param inFolderPath path to the packages directory to traverse
 * @returns true if successful, false otherwise.
*/
bool			CSL_LoadCSL(const char *inFolderPath);


/** CSL_MatchPlane finds a CSL that matches the specified PlaneType.
 *
 * Given an ICAO and optionally a livery and airline, this routine returns the best plane match, or
 * NULL if there is no good plane match.
 *
 * if match_quality is set, it is set with the pass upon which a match was determined.  
 *   (see XPMPMultiplayerCSL.h)
 */
CSL *			CSL_MatchPlane(const PlaneType &type,int *match_quality, bool allow_default);

/*
 * CSL_Dump
 *
 * This routine dumps the total state of the CSL system to the error.out file.
 *
 */
void			CSL_Dump();

#endif /* XPLMMULTIPLAYERCSL_H */
