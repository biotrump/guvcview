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

#ifndef CORE_IO_H
#define CORE_IO_H

/*
 * smart concatenation
 * args:
 *    dest - destination string
 *    c - connector char
 *    str1 - string to concat
 *
 * asserts:
 *    none
 *
 * returns: concatenated string (must free)
 */
char *smart_cat(const char *dest, char c, const char *str1);

/*
 * get the filename basename
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with basename (must free it)
 */
char *get_file_basename(const char *filename);

/*
 * get the filename path
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with path (must free it)
 *      or NULL if no path found
 */
char *get_file_pathname(const char *filename);

/*
 * get the filename extension
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with extension (must free it)
 *      or NULL if no extension found
 */
char *get_file_extension(const char *filename);

/*
 * change the filename extension
 * args:
 *    filename - string with filename
 *    ext - string with new extension
 *
 * asserts:
 *    none
 *
 * returns: string with new extension (must free it)
 */
char *set_file_extension(const char *filename, const char *ext);

#endif