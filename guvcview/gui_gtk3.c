/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           Flemming Frandsen <dren.dk@gmail.com>                               #
#                             Add VU meter OSD                                  #
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gview.h"
#include "gui.h"
#include "gui_gtk3.h"
#include "gui_gtk3_callbacks.h"

extern int debug_level;

/* The main window*/
static GtkWidget *main_window;

/*
 * GUI initialization
 * args:
 *   device - pointer to device data we want to attach the gui for
 *   gui - gui API to use (GUI_NONE, GUI_GTK3, ...)
 *   width - window width
 *   height - window height
 *
 * asserts:
 *   device is not null
 *
 * returns: error code (0 -OK)
 */
int gui_attach_gtk3(v4l2_dev *device, int width, int height)
{
	/*asserts*/
	assert(device != NULL);

	if(!gtk_init_check(&argc, &argv))
	{
		fprintf(stderr, "GUVCVIEW: (GUI) Gtk3 can't open display\n");
		return -1;
	}

	g_set_application_name(_("Guvcview Video Capture"));

	/* make sure the type is realized so that we can change the properties*/
	g_type_class_unref (g_type_class_ref (GTK_TYPE_BUTTON));
	/* make sure gtk-button-images property is set to true (defaults to false in karmic)*/
	g_object_set (gtk_settings_get_default (), "gtk-button-images", TRUE, NULL);

	/* Create a main window */
	main_windown = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (main_window), _("Guvcview"));

	/* get screen resolution */
	GdkScreen* screen = NULL;
	screen = gtk_window_get_screen(GTK_WINDOW(main_window));
	int desktop_width = gdk_screen_get_width(screen);
	int desktop_height = gdk_screen_get_height(screen);

	if(debug_level > 0)
		printf("GUVCVIEW: (GUI) Screen resolution is (%d x %d)\n", desktop_width, desktop_height);

	if((width > desktop_width) && (desktop_width > 0))
		width = desktop_width;
	if((height > desktop_height) && (desktop_height > 0))
		height = desktop_height;

	gtk_window_resize(GTK_WINDOW(main_window), width, height);

	/* Add delete event handler */
	g_signal_connect(GTK_WINDOW(main_window), "delete_event", G_CALLBACK(delete_event), NULL);

	/*----------------------- Main table --------------------------------------*/
	GtkWidget *maintable = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	gtk_widget_show (maintable);

	/*----------------------- Top Menu ----------------------------------------*/
	GtkWidget *menubar = gtk_menu_bar_new();
	gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar), GTK_PACK_DIRECTION_LTR);

	GtkWidget *controls_menu = gtk_menu_new();
	GtkWidget *controls_top = gtk_menu_item_new_with_label(_("Settings"));
	GtkWidget *controls_load = gtk_menu_item_new_with_label(_("Load Profile"));
	GtkWidget *controls_save = gtk_menu_item_new_with_label(_("Save Profile"));
	GtkWidget *controls_default = gtk_menu_item_new_with_label(_("Hardware Defaults"));

	gtk_widget_show (controls_top);
	gtk_widget_show (controls_load);
	gtk_widget_show (controls_save);
	gtk_widget_show (controls_default);

	GtkWidget *camera_button_menu = gtk_menu_new();
	GtkWidget *camera_button_top = gtk_menu_item_new_with_label(_("Camera Button"));

	GSList *camera_button_group = NULL;
	GtkWidget *def_image = gtk_radio_menu_item_new_with_label(camera_button_group, _("Capture Image"));
	g_object_set_data (G_OBJECT (def_image), "camera_default", GINT_TO_POINTER(0));
	gtk_menu_shell_append(GTK_MENU_SHELL(camera_button_menu), def_image);

	camera_button_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (def_image));
	GtkWidget *def_video = gtk_radio_menu_item_new_with_label(camera_button_group, _("Capture Video"));
	g_object_set_data (G_OBJECT (def_video), "camera_default", GINT_TO_POINTER(1));
	gtk_menu_shell_append(GTK_MENU_SHELL(camera_button_menu), def_video);

	gtk_widget_show (camera_button_top);
	gtk_widget_show (def_image);
	gtk_widget_show (def_video);

	/*default camera button action*/
	if (get_default_camera_button_action() == 0)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (def_image), TRUE);
	else
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (def_video), TRUE);

	g_signal_connect (GTK_RADIO_MENU_ITEM(def_image), "toggled",
		G_CALLBACK (camera_button_menu_changed), NULL);
	g_signal_connect (GTK_RADIO_MENU_ITEM(def_video), "toggled",
		G_CALLBACK (camera_button_menu_changed), NULL);

	/* profile events*/
	g_object_set_data (G_OBJECT (controls_save), "profile_dialog", GINT_TO_POINTER(1));
	g_signal_connect (GTK_MENU_ITEM(controls_save), "activate",
		G_CALLBACK (controls_profile_clicked), device);
	g_object_set_data (G_OBJECT (controls_load), "profile_dialog", GINT_TO_POINTER(0));
	g_signal_connect (GTK_MENU_ITEM(controls_load), "activate",
		G_CALLBACK (controls_profile_clicked), device);
	g_signal_connect (GTK_MENU_ITEM(controls_default), "activate",
		G_CALLBACK (control_defaults_clicked), device);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(controls_top), controls_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), controls_load);
	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), controls_save);
	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), controls_default);
	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), camera_button_top);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), controls_top);

	/*control_only exclusion */




	gtk_widget_show (menubar);
	gtk_container_set_resize_mode (GTK_CONTAINER(menubar), GTK_RESIZE_PARENT);

	/* Attach the menu */
	gtk_box_pack_start(GTK_BOX(maintable), menubar, FALSE, TRUE, 2);

	/*----------------------- Image controls Tab ------------------------------*/
	GtkWidget *img_controls_grid = gtk_grid_new();

	gtk_grid_set_column_homogeneous (GTK_GRID(img_controls_grid), FALSE);
	gtk_widget_set_hexpand (img_controls_grid, TRUE);
	gtk_widget_set_halign (img_controls_grid, GTK_ALIGN_FILL);

	gtk_grid_set_row_spacing (GTK_GRID(img_controls_grid), 4);
	gtk_grid_set_column_spacing (GTK_GRID (img_controls_grid), 4);
	gtk_container_set_border_width (GTK_CONTAINER (img_controls_grid), 2);

	int i = 0;
	v4l2_ctrl* current = device->list_device_controls;
    v4l2_ctrl* next = current->next;
    for(; next != NULL; current = next, next = current->next)
    {
		/*label*/
		char *tmp;
        tmp = g_strdup_printf ("%s:", gettext((char *) current->control.name));
        GtkWidget *label = gtk_label_new (tmp);
        free(tmp);
        gtk_widget_show (label);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

        GtkWidget *widget = NULL;
        GtkWidget *widget2 = NULL; /*usually a spin button*/

		switch (current->control.type)
		{
			case V4L2_CTRL_TYPE_INTEGER:
				//printf("control[%d]:(int) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				//printf("\tmin:%d max:%d step:%d def:%d curr:%d\n",
				//	control->control.minimum, control->control.maximum, control->control.step,
				//	control->control.default_value, control->value);
				switch (current->control.id)
				{
					//special cases
					case V4L2_CID_PAN_RELATIVE:
					case V4L2_CID_TILT_RELATIVE:
					{
						widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);

						GtkWidget *PanTilt1 = NULL;
						GtkWidget *PanTilt2 = NULL;
						if(current->control.id == V4L2_CID_PAN_RELATIVE)
						{
							PanTilt1 = gtk_button_new_with_label(_("Left"));
							PanTilt2 = gtk_button_new_with_label(_("Right"));
						}
						else
						{
							PanTilt1 = gtk_button_new_with_label(_("Down"));
							PanTilt2 = gtk_button_new_with_label(_("Up"));
						}

						gtk_widget_show (PanTilt1);
						gtk_widget_show (PanTilt2);
						gtk_box_pack_start(GTK_BOX(widget),PanTilt1,TRUE,TRUE,2);
						gtk_box_pack_start(GTK_BOX(widget),PanTilt2,TRUE,TRUE,2);

						g_object_set_data (G_OBJECT (PanTilt1), "control_info",
							GINT_TO_POINTER(current->control.id));
						g_object_set_data (G_OBJECT (PanTilt2), "control_info",
							GINT_TO_POINTER(current->control.id));

						g_signal_connect (GTK_BUTTON(PanTilt1), "clicked",
							G_CALLBACK (button_PanTilt1_clicked), device);
						g_signal_connect (GTK_BUTTON(PanTilt2), "clicked",
							G_CALLBACK (button_PanTilt2_clicked), device);

						gtk_widget_show (widget);

						widget2 = gtk_spin_button_new_with_range(-256, 256, 64);

						gtk_editable_set_editable(GTK_EDITABLE(widget2), TRUE);

						gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget2), 128);

						g_signal_connect(GTK_SPIN_BUTTON(widget2),"value-changed",
							G_CALLBACK (pantilt_step_changed), device);

						gtk_widget_show (widget2);

						break;
					}

					case V4L2_CID_PAN_RESET:
					case V4L2_CID_TILT_RESET:
					{
						widget = gtk_button_new_with_label(" ");
						gtk_widget_show (widget);

						g_object_set_data (G_OBJECT (widget), "control_info",
							GINT_TO_POINTER(current->control.id));

						g_signal_connect (GTK_BUTTON(widget), "clicked",
							G_CALLBACK (button_clicked), all_data);

						break;
					};

					case V4L2_CID_LED1_MODE_LOGITECH:
					{
						char* LEDMenu[4] = {_("Off"),_("On"),_("Blinking"),_("Auto")};
						/*turn it into a menu control*/
						if(!current->menu)
                    		current->menu = calloc(4+1, sizeof(struct v4l2_querymenu));
                    	else
                    		current->menu = realloc(current->menu,  (4+1) * sizeof(struct v4l2_querymenu));

						current->menu[0].id = current->control.id;
						current->menu[0].index = 0;
						current->menu[0].name[0] = 'N'; //just set something here
						current->menu[1].id = current->control.id;
						current->menu[1].index = 1;
						current->menu[1].name[0] = 'O';
						current->menu[2].id = current->control.id;
						current->menu[2].index = 2;
						current->menu[2].name[0] = 'B';
						current->menu[3].id = current->control.id;
						current->menu[3].index = 3;
						current->menu[3].name[0] = 'A';
						current->menu[4].id = current->control.id;
						current->menu[4].index = current->control.maximum+1;
						current->menu[4].name[0] = '\0';

						int j = 0;
						int def = 0;

						widget = gtk_combo_box_text_new ();
						for (j = 0; current->menu[j].index <= current->control.maximum; j++)
						{
							gtk_combo_box_text_append_text (
								GTK_COMBO_BOX_TEXT (widget),
								(char *) LEDMenu[j]);
							if(current->value == current->menu[j].index)
								def = j;
						}

						gtk_combo_box_set_active (GTK_COMBO_BOX(widget), def);
						gtk_widget_show (widget);

						g_object_set_data (G_OBJECT (widget), "control_info",
                           	GINT_TO_POINTER(current->control.id));

						g_signal_connect (GTK_COMBO_BOX_TEXT(current->widget), "changed",
							G_CALLBACK (combo_changed), all_data);

						break;
					}

                        case V4L2_CID_RAW_BITS_PER_PIXEL_LOGITECH:
                        {
                            /*turn it into a menu control*/
                            char* BITSMenu[2] = {_("8 bit"),_("12 bit")};
                            /*turn it into a menu control*/
                            if(!current->menu)
                    			current->menu = g_new0(struct v4l2_querymenu, 2+1);
                    		else
                    			current->menu = g_renew(struct v4l2_querymenu, current->menu, 2+1);

                   			current->menu[0].id = current->control.id;
                   			current->menu[0].index = 0;
                   			current->menu[0].name[0] = 'o'; //just set something here
                   			current->menu[1].id = current->control.id;
                   			current->menu[1].index = 1;
                   			current->menu[1].name[0] = 'd';
                   			current->menu[2].id = current->control.id;
                   			current->menu[2].index = 2;
                   			current->menu[2].name[0] = '\0';
                            int j = 0;
                        	int def = 0;
                        	current->widget = gtk_combo_box_text_new ();
                        	for (j = 0; current->menu[j].index <= current->control.maximum; j++)
                        	{
                        		//if (verbose)
        	                   	//	printf("adding menu entry %d: %d, %s\n",j, current->menu[j].index, current->menu[j].name);
                            	gtk_combo_box_text_append_text (
                                	GTK_COMBO_BOX_TEXT (current->widget),
                                	(char *) BITSMenu[j]);
                            	if(current->value == current->menu[j].index)
                            		def = j;
                        	}

                        	gtk_combo_box_set_active (GTK_COMBO_BOX(current->widget), def);
                        	gtk_widget_show (current->widget);

                        	g_object_set_data (G_OBJECT (current->widget), "control_info",
                            	GINT_TO_POINTER(current->control.id));
                        	//connect signal
                       	 	g_signal_connect (GTK_COMBO_BOX_TEXT(current->widget), "changed",
                            	G_CALLBACK (combo_changed), all_data);
                        };
                        break;

                        default: //standard case - hscale
                        {
                            /* check for valid range */
                            if((current->control.maximum > current->control.minimum) && (current->control.step != 0))
                            {
                            	GtkAdjustment *adjustment =  gtk_adjustment_new (
                                	current->value,
                                	current->control.minimum,
                                    current->control.maximum,
                                    current->control.step,
                                    current->control.step*10,
                                    0);

                                current->widget = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adjustment);

                                gtk_scale_set_draw_value (GTK_SCALE (current->widget), FALSE);
                                gtk_range_set_round_digits(GTK_RANGE (current->widget), 0);

                                gtk_widget_show (current->widget);

                                current->spinbutton = gtk_spin_button_new(adjustment,current->control.step, 0);

                                /*can't edit the spin value by hand*/
                                gtk_editable_set_editable(GTK_EDITABLE(current->spinbutton),TRUE);

                                gtk_widget_show (current->spinbutton);

                                g_object_set_data (G_OBJECT (current->widget), "control_info",
                                    GINT_TO_POINTER(current->control.id));
                                g_object_set_data (G_OBJECT (current->spinbutton), "control_info",
                                    GINT_TO_POINTER(current->control.id));
                                //connect signal
                                g_signal_connect (GTK_SCALE(current->widget), "value-changed",
                                    G_CALLBACK (slider_changed), all_data);
                                g_signal_connect(GTK_SPIN_BUTTON(current->spinbutton),"value-changed",
                                    G_CALLBACK (spin_changed), all_data);
                            }
                            else
                            {
                                printf("INVALID RANGE (MAX <= MIN) for control id: 0x%08x \n", current->control.id);
                            }
                        };
                        break;
                    };
                };
                break;

#ifdef V4L2_CTRL_TYPE_INTEGER64
			case V4L2_CTRL_TYPE_INTEGER64:
				printf("control[%d]:(int64) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				printf ("\tcurr:%" PRIu64 "\n", control->value64);
				break;
#endif
#ifdef V4L2_CTRL_TYPE_STRING
			case V4L2_CTRL_TYPE_STRING:
				printf("control[%d]:(str) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				printf ("\tmin:%d max:%d step:%d\n",
					control->control.minimum, control->control.maximum, control->control.step);
				break;
#endif
			case V4L2_CTRL_TYPE_BOOLEAN:
				printf("control[%d]:(bool) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				printf ("\tdef:%d curr:%d\n",
					control->control.default_value, control->value);
				break;

			case V4L2_CTRL_TYPE_MENU:
				printf("control[%d]:(menu) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				printf("\tmin:%d max:%d def:%d curr:%d\n",
					control->control.minimum, control->control.maximum,
					control->control.default_value, control->value);
				for (j = 0; control->menu[j].index <= control->control.maximum; j++)
					printf("\tmenu[%d]: [%d] -> '%s'\n", j, control->menu[j].index, control->menu[j].name);
				break;

#ifdef V4L2_CTRL_TYPE_INTEGER_MENU
			case V4L2_CTRL_TYPE_INTEGER_MENU:
				printf("control[%d]:(intmenu) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				printf("\tmin:%d max:%d def:%d curr:%d\n",
					control->control.minimum, control->control.maximum,
					control->control.default_value, control->value);
				for (j = 0; control->menu[j].index <= control->control.maximum; j++)
					printf("\tmenu[%d]: [%d] -> %" PRId64 " (0x%" PRIx64 ")", j, control->menu[j].index,
						(int64_t) control->menu[j].value,
						(int64_t) control->menu[j].value);
				break;
#endif
			case V4L2_CTRL_TYPE_BUTTON:
				printf("control[%d]:(button) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				break;

#ifdef V4L2_CTRL_TYPE_BITMASK
			case V4L2_CTRL_TYPE_BITMASK:
				printf("control[%d]:(bitmask) 0x%x '%s'\n",i ,control->control.id, control->control.name);
				printf("\tmax:%d def:%d curr:%d\n",
					control->control.maximum, control->control.default_value, control->value);
#endif
			default:
				printf("control[%d]:(unknown - 0x%x) 0x%x '%s'\n",i ,control->control.type,
					control->control.id, control->control.name);
				break;
		}
        i++;
    }
    /*last one*/
	print_control(current, i);


	return 0;
}

/*
 * run the GUI loop
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int gui_run_gtk3()
{

	int ret = 0;



	return ret;
}

/*
 * closes and cleans the GTK3 GUI
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void gui_close_gtk3()
{
	gtk_main_quit();
}