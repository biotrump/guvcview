/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
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

#ifndef VIDEO_FILTERS_H
#define VIDEO_FILTERS_H

#include "defs.h"

/* Flip YUYV frame - horizontal
 * args:
 *      frame = pointer to frame buffer (yuyv format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
yuyv_mirror (BYTE *frame, int width, int height);

/* Flip UYVY frame - horizontal
 * args:
 *      frame = pointer to frame buffer (uyvy format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
uyvy_mirror (BYTE *frame, int width, int height);

/* Flip YUV frame - vertical
 * args:
 *      frame = pointer to frame buffer (yuyv or uyvy format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void  
yuyv_upturn(BYTE* frame, int width, int height);

/* Invert YUV frame
 * args:
 *      frame = pointer to frame buffer (yuyv or uyvy format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
yuyv_negative(BYTE* frame, int width, int height);

/* monochromatic effect for YUYV frame
 * args:
 *      frame = pointer to frame buffer (yuyv format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
yuyv_monochrome(BYTE* frame, int width, int height);

/* monochromatic effect for UYVY frame
 * args:
 *      frame = pointer to frame buffer (uyvy format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
uyvy_monochrome(BYTE* frame, int width, int height);

/*break image in little square pieces
 * args:
 *    frame  = pointer to frame buffer (yuyv or uyvy format)
 *    width  = frame width
 *    height = frame height
 *    piece_size = multiple of 2 (we need at least 2 pixels to get the entire pixel information)
 *    format = v4l2 pixel format
 */
void
pieces(BYTE* frame, int width, int height, int piece_size, int format);

#endif