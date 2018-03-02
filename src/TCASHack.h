//
// Created by kuroneko on 2/03/2018.
//

#ifndef XPMP_TCASHACK_H
#define XPMP_TCASHACK_H

#include <vector>

#include <XPLMDataAccess.h>

/* Maximum altitude difference in feet for TCAS blips */
#define		MAX_TCAS_ALTDIFF		10000

/* TCAS datarefs */
extern std::vector<XPLMDataRef>			gMultiRef_X;
extern std::vector<XPLMDataRef>			gMultiRef_Y;
extern std::vector<XPLMDataRef>			gMultiRef_Z;

/** TCAS_Init prepares the TCAS hack subsystem
 *
 * @return number of multiplayer aircraft positions we were able to get datarefs for.
 */
int		TCAS_Init();

/** TCAS_Enable starts the TCAS evaluation callbacks */
void	TCAS_Enable();

/** TCAS_Disable stops the TCAS evaluation callbacks and disables the TCAS data injection if it was enabled */
void	TCAS_Disable();

#endif //XPMP_TCASHACK_H
