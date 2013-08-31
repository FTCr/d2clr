/*
 * Copyright 2013 FIL
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#define NOTIFY_H_MSG_RU "Состояние подбора игры"
#define NOTIFY_B_MSG_RU "Ваша игра готова"
#define DBUS_RULE "eavesdrop=true,type='method_call'"

#define HELP "Usage d2clr [OPTIONS]...\n" \
"Actions:\n" \
"  -x,-X                   X \"Accept\" button coordinate.\n" \
"  -y,-Y                   Y \"Accept\" button coordinate.\n" \
"  -h                      Print help.\n"

typedef struct
{
	Display *display;
	int x;
	int y;
} m_data;

DBusHandlerResult signal_filter(DBusConnection *connection, DBusMessage *msg, void *user_data)
{
	if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "Notify"))
	{
		DBusMessageIter iter;
		dbus_message_iter_init(msg, &iter);
		
		int type = dbus_message_iter_get_arg_type(&iter);
		
		char *val;
		for (int i = 0; i < 3; i++)
			dbus_message_iter_next (&iter);
		dbus_message_iter_get_basic (&iter, &val);
		
		if (strcmp(val, NOTIFY_H_MSG_RU) == 0) 
		{
			
			dbus_message_iter_next (&iter);
			dbus_message_iter_get_basic (&iter, &val);
			
			if (strcmp(val, NOTIFY_B_MSG_RU) == 0)
			{
				m_data *data = (m_data*)user_data;
				//activated Dota 2 window
				sleep(1);
				system("wmctrl -a \"DOTA 2 - OpenGL\"");
				sleep(1);
				//mouse move
				Window root = DefaultRootWindow(data->display);
				XWarpPointer(data->display, None, root, 0, 0, 0, 0, data->x, data->y);
				//mouse click
				for (int i = 0; i < 3; i++)
				{
					XTestFakeButtonEvent(data->display, 1, True, CurrentTime);
					XFlush(data->display);
					usleep(200000);
					XTestFakeButtonEvent(data->display, 1, False, CurrentTime);
					XFlush(data->display);
					usleep(500000); //0.5 sec
				}
			}
		}
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}

int main(int argc, char **argv)
{
	if (argc <= 4)
	{
		fputs(HELP, stdout);
		return EXIT_FAILURE;
	}
	
	m_data data;
	
	int opt = 0;
	//opterr = 0;
	while ((opt = getopt(argc,argv,"X:x:Y:y:h:")) != -1)
	{
		switch (opt)
		{
			case 'X': case 'x':
				sscanf(optarg, "%d", &data.x);
			break;
			case 'Y': case 'y':
				sscanf(optarg, "%d", &data.y);
			break;
			case 'h': case '?':
				fputs(HELP, stdout);
			return EXIT_FAILURE;
        }
	}
	
	data.display = XOpenDisplay(NULL);
	if (!data.display)
	{
		g_error("Cannot open display.\n");
		return EXIT_FAILURE;
	}
	
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	
	DBusError error;
	dbus_error_init(&error);
	
	DBusConnection *connect = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (dbus_error_is_set(&error))
	{
		g_error("Cannot get System BUS Connection: %s", error.message);
		dbus_error_free(&error);
		return EXIT_FAILURE;
	}
	
	dbus_connection_setup_with_g_main(connect, NULL);
	
	dbus_bus_add_match(connect, DBUS_RULE, &error);
	
	if (dbus_error_is_set(&error))
	{
		g_error("Cannot add D-BUS match rule, cause: %s", error.message);
		dbus_error_free(&error);
		return EXIT_FAILURE;
	}
	
	g_message("Listening D-BUS");
	
	dbus_connection_add_filter(connect, signal_filter, &data, NULL);
	g_main_loop_run(loop);
	
	XCloseDisplay(data.display);
	
	return EXIT_SUCCESS; 
}