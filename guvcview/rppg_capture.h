/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/
#ifndef RPPG_CAPTURE_H
#define RPPG_CAPTURE_H

#include <inttypes.h>
#include <sys/types.h>

#include "gviewaudio.h"

typedef struct _rppg_loop_data_t
{
	void *options;
	void *config;
	void *device;
} rppg_loop_data_t;

/*
 * checks if video timed capture is on
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: 1 if on; 0 if off
 */
int check_rppg_timer();

/*
 * reset video timer
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void reset_rppg_timer();

/*
 * quit callback
 * args:
 *    data - pointer to user data
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int quit_callback(void *data);

/*
 * start the rppg thread
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int start_rppg_thread();

/*
 * stop the encoder thread
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int stop_rppg_thread();

/*
 * capture loop (should run in a separate thread)
 * args:
 *    data - pointer to user data
 *
 * asserts:
 *    device data is not null
 *
 * returns: pointer to return code
 */
void *capture_rppg_loop(void *data);

#endif
