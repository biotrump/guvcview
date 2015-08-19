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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gviewv4l2core.h"
#include "gviewrender.h"
#include "gviewencoder.h"
#include "gview.h"
#include "rppg_capture.h"
#include "options.h"
#include "config.h"
#include "core_io.h"
#include "gui.h"
#include "../config.h"
#include "bcv-export.h"

/*flags*/
extern int debug_level;

static int quit = 0; /*terminate flag*/
static int save_trace = 0; /*save video flag*/
static int rppg_start = 0;
static int restart = 0; /*restart flag*/

static __THREAD_TYPE rppg_thread;

static int my_rppg_status = 0;

static char status_message[80];

/*
 * sets the save video flag
 * args:
 *    value - save_trace flag value
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void rppg_capture_save_trace(int value)
{
	save_trace = value;

	if(debug_level > 1)
		printf("GUVCVIEW: save video flag changed to %i\n", save_trace);
}

/*
 * gets the save video flag
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: save_trace flag
 */
int rppg_capture_get_save_trace()
{
	return save_trace;
}

/*
 * get encoder started flag
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: encoder started flag (1 -started; 0 -not started)
 */
int get_rppg_status()
{
	return my_rppg_status;
}

/*
 * rppg loop (should run in a separate thread)
 * args:
 *    data - pointer to user data
 *
 * asserts:
 *   none
 *
 * returns: pointer to return code
 */
static void *rppg_loop(void *data)
{
	my_rppg_status = 1;

	if(debug_level > 1)
		printf("GUVCVIEW: rppg thread (tid: %u)\n",
			(unsigned int) syscall (SYS_gettid));

	char *rppg_filename = NULL;
	/*get_video_[name|path] always return a non NULL value*/
	char *name = strdup(get_rppg_name());
	char *path = strdup(get_rppg_path());

	if(get_rppg_sufix_flag())
	{
		char *new_name = add_file_suffix(path, name);
		free(name); /*free old name*/
		name = new_name; /*replace with suffixed name*/
	}
	int pathsize = strlen(path);
	if(path[pathsize] != '/')
		rppg_filename = smart_cat(path, '/', name);
	else
		rppg_filename = smart_cat(path, 0, name);

	snprintf(status_message, 79, _("saving rppg to %s"), rppg_filename);
	gui_status_message(status_message);

	/*start rppg capture*/
	rppg_capture_save_trace(1);	//save raw trace
	rppg_capture_start(1);		//start rppg

	int treshold = 102400; /*100 Mbytes*/
	int64_t last_check_pts = 0; /*last pts when disk supervisor called*/

	while(rppg_capture_get_save_trace())
	{

		/*
		* no buffers to process
		* sleep a couple of milisec
		*/
		struct timespec req = {
		.tv_sec = 0,
		.tv_nsec = 1000000};/*nanosec*/
		nanosleep(&req, NULL);

#if 0
		/*process the video buffer*/
		if(encoder_process_next_video_buffer(encoder_ctx) > 0)
		{
			/*
			 * no buffers to process
			 * sleep a couple of milisec
			 */
			 struct timespec req = {
				.tv_sec = 0,
				.tv_nsec = 1000000};/*nanosec*/
			 nanosleep(&req, NULL);

		}

		/*disk supervisor*/
		if(encoder_ctx->enc_video_ctx->pts - last_check_pts > 2 * NSEC_PER_SEC)
		{
			last_check_pts = encoder_ctx->enc_video_ctx->pts;

			if(!encoder_disk_supervisor(treshold, path))
			{
				/*stop capture*/
				gui_set_rppg_capture_button_status(0);
			}
		}
#endif
	}

	/*clean string*/
	free(rppg_filename);
	free(path);
	free(name);

	my_rppg_status = 0;

	return ((void *) 0);
}

/*
 * start the encoder thread
 * args:
 *   data - pointer to user data
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int start_rppg_thread(void *data)
{
	int ret = __THREAD_CREATE(&rppg_thread, rppg_loop, data);

	if(ret){
		fprintf(stderr, "GUVCVIEW: rppg thread creation failed (%i)\n", ret);
	}else{
		bcvMeasureEvent(MEASURE_EVENT_START);	//start measured

		if(debug_level > 2)
		printf("GUVCVIEW: created rppg thread with tid: %u\n",
			(unsigned int) rppg_thread);
	}
	return ret;
}

/*
 * stop the rppg thread
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int stop_rppg_thread()
{
	///////////////////////////////
	bcvMeasureEvent(MEASURE_EVENT_STOP);	//stop measured
	////////////////////////////////////
	rppg_capture_save_trace(0);
	rppg_capture_start(0);

	__THREAD_JOIN(rppg_thread);

	return 0;
}
