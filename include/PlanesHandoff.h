//
// Created by chris on 19/04/2020.
//

#ifndef XPMP_PLANESHANDOFF_H
#define XPMP_PLANESHANDOFF_H

#ifndef XPMP_CLIENT_NAME
#define XPMP_CLIENT_NAME "AClient"
#endif

/** PLANES_SAFEACQURIE_DREF_NAME is the name of the dataref we use to coordinate
 * handover of the aircraft state mutex.  This must not change as it has to be
 * uniform for all plugins participating in this scheme.
 */
#define PLANES_SAFEACQUIRE_DREF_NAME    "xplanemp/want_planes"

/** PLANES_SAFEACQUIRE_OK is returned when the operation completed successfully
 */
#define PLANES_SAFEACQUIRE_OK (0)


/** PLANES_SAFEACQUIRE_ERROR is returned when an unrecoverable error occurs.
 */
#define PLANES_SAFEACQUIRE_ERROR (-1)

/** Take Only From Passive will only acquire the lock if the existing parties have not
 * "claimed" the mutex.  ("Passive" consumers and no consumer).  Default behaviour
 * is to try to take it from anybody.
 */
#define PLANES_SAFEACQUIRE_TAKE_ONLY_FROM_PASSIVE (1U)

/** Passive Only has the Take-from-passive behaviour, except we also will not
 * explicitly claim the mutex, but yield it to any party who wants it.
 */
#define PLANES_SAFEACQUIRE_PASSIVE_ONLY (2U)

/** do not install a lingering acquisition callback.  If we fail to acquire with
 * NOWAIT, we also return a non-recoverable error state.
 */
#define PLANES_SAFEACQUIRE_NOWAIT (4U)

#ifdef __cplusplus
extern "C" {
#endif

/** StateCallback_f is the function prototype for the callbacks that will
 * be called by Planes_SafeAcquire when acquisition successfully occurs, or when
 * we have to release control.
 */
typedef void (*StateCallback_f)(void *refcon);


/** Planes_SafeAcquire tries to acquire the MP aircraft.
 *
 * @param acquirePtr the callback to invoke when multiplayer acquisition
 * successfully occurs.
 *
 * @note There can only be one acquirePtr in any one single instance of this
 *      stub at any time.  If you call Planes_SafeAcquire with different
 *      function pointers, only the most recent one will be used!  You probably
 *      don't want to do that anyway.
 *
 * @note Actually, if you call SafeAcquire whilst there's an outstanding
 *      acquisition attempt, that is an error and it will be reported as such.
 *
 * @param refcon the opaque pointer to hand to the callback.
 * @return 0 if we've been able to safely enter the queue.  !0 if a non-recoverable error occured.
 */
int
Planes_SafeAcquire(StateCallback_f acquirePtr,
                   StateCallback_f releasePtr,
                   void *refcon,
                   unsigned int flags);

/** Planes_SafeRelease releases the MP aircraft and ceases any outstanding
 * attempts to acquire.
 */
void
Planes_SafeRelease(void);

/** Nobody has the planes mutex right now */
#define PLANES_SAFEACQUIRE_STATE_UNCLAIMED  (0)

/** The planes mutex is held by a passive or non-cooperating consumer. */
#define PLANES_SAFEACQUIRE_STATE_PASSIVE_CONSUMER  (1)

/** THe planes mutex is held by an active cooperating consumer */
#define PLANES_SAFEACQUIRE_STATE_ACTIVE_CONSUMER  (2)

/** The planes mutex is held by a non-cooperating consumer or is in the middle
 * of a handover. */
#define PLANES_SAFEACQUIRE_STATE_LEGACY_CONSUMER  (3)

/** Planes_AcquisitionStatus works out what the current state of affairs is with
 * the XPLM Planes mutex.
 *
 * It tells us if the mutex is free, if there's a passive consumer, or an active
 * consumer, a non-yielded/incompatible consumer (see the note) and optionally
 * identifies who the consumer is.
 *
 * @note The non-yielded/incompatible consumer state is only safe to interpret
 *   outside of any shared dataref callback chains on
 *   PLANES_SAFEACQUIRE_DREF_NAME.  During that callback chain, the handover
 *   could still be in progress, and any such result of this type incorrect.
 *
 * @param outControllingPlugin If set, *outControllingPlugin will be set to the
 *      XPLMPluginID of the plugin controlling the multiplayer aircraft, or
 *      XPLM_NO_PLUGIN_ID if there is no consumer (ie: X-Plane has control).
 *
 * @return one of PLANES_SAFEACQUIRE_STATE_UNCLAIMED,
 *      PLANES_SAFEACQUIRE_STATE_PASSIVE_CONSUMER,
 *      PLANES_SAFEACQUIRE_STATE_ACTIVE_CONSUMER, or
 *      PLANES_SAFEACQUIRE_STATE_LEGACY_CONSUMER
 */
int
Planes_AcquisitionStatus(XPLMPluginID *outControllingPlugin);

#ifdef __cplusplus
}
#endif

#endif //XPMP_PLANESHANDOFF_H
