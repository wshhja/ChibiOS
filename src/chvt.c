/*
    ChibiOS/RT - Copyright (C) 2006-2007 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file chvt.c
 * @brief Time related code.
 * @addtogroup Time
 * @{
 */

#include <ch.h>

VTList vtlist;

/**
 * @brief Virtual Timers initialization.
 *
 * @note Internal use only.
 */
void vt_init(void) {

  vtlist.vt_next = vtlist.vt_prev = (void *)&vtlist;
  vtlist.vt_time = (systime_t)-1;
  vtlist.vt_systime = 0;
}

/**
 * @brief Enables a virtual timer.
 *
 * @param vtp the @p VirtualTimer structure pointer
 * @param time the number of time ticks, the value zero is not allowed
 * @param vtfunc the timer callback function. After invoking the callback
 *               the timer is disabled and the structure can be disposed or
 *               reused.
 * @param par a parameter that will be passed to the callback function
 * @note The associated function is invoked by an interrupt handler within
 *       the I-Locked state, see @ref system_states.
 */
void chVTSetI(VirtualTimer *vtp, systime_t time, vtfunc_t vtfunc, void *par) {
  VirtualTimer *p;

  chDbgCheck((vtp != NULL) && (time != 0) && (vtfunc != NULL), "chVTSetI");

  vtp->vt_par = par;
  vtp->vt_func = vtfunc;
  p = vtlist.vt_next;
  while (p->vt_time < time) {
    time -= p->vt_time;
    p = p->vt_next;
  }

  vtp->vt_prev = (vtp->vt_next = p)->vt_prev;
  vtp->vt_prev->vt_next = p->vt_prev = vtp;
  vtp->vt_time = time;
  if (p != (void *)&vtlist)
    p->vt_time -= time;
}

/**
 * @brief Disables a Virtual Timer.
 *
 * @param vtp the @p VirtualTimer structure pointer
 * @note The timer MUST be active when this function is invoked.
 */
void chVTResetI(VirtualTimer *vtp) {

  chDbgCheck(vtp != NULL, "chVTResetI");
  chDbgAssert(vtp->vt_func != NULL, "chVTResetI(), #1",
              "timer already triggered");

  if (vtp->vt_next != (void *)&vtlist)
    vtp->vt_next->vt_time += vtp->vt_time;
  vtp->vt_prev->vt_next = vtp->vt_next;
  vtp->vt_next->vt_prev = vtp->vt_prev;
  vtp->vt_func = NULL;
}

/**
 * @brief Checks if the current system time is within the specified time window.
 *
 * @param start the start of the time window (inclusive)
 * @param end the end of the time window (non inclusive)
 */
bool_t chSysInTimeWindow(systime_t start, systime_t end) {

  systime_t time = chSysGetTime();
  return end >= start ? (time >= start) && (time < end) :
                        (time >= start) || (time < end);
}

/** @} */
