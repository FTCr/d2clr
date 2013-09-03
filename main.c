/*
 * Copyright FIL 2013
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
#include <getopt.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#define NOTIFY_H_MSG_RU "Состояние подбора игры"
#define NOTIFY_B_MSG_RU "Ваша игра готова"
#define NOTIFY_H_MSG_EN "Matchmaking Status"
#define NOTIFY_B_MSG_EN "Your game is ready"
#define NOTIFY_H_MSG_DE "Matchmaking-Status"
#define NOTIFY_B_MSG_DE "Ihr Spiel ist bereit"
#define NOTIFY_H_MSG_CS "Stav vyhledávání zápasů"
#define NOTIFY_B_MSG_CS "Tvá hra je připravena"
#define NOTIFY_H_MSG_FR "État de la création de match"
#define NOTIFY_B_MSG_FR "Votre partie est prête"



#define DBUS_RULE "eavesdrop=true,type='method_call'"

#define HELP "Usage: d2clr [OPTIONS]...\n" \
"Actions:\n" \
"  -x, -X                   X \"Accept\" button coordinate.\n" \
"  -y, -Y                   Y \"Accept\" button coordinate.\n" \
"  -l, --lang               Language \"Dota 2\" game. ISO 639-1 codes.\n" \
"  -h, --help               Print help.\n"

enum
{
	en,
	ru,
	de,
	cs,
	fr
} lang;

typedef struct
{
	Display *display;
	int x;
	int y;
	int lang;
} m_data;

DBusHandlerResult signal_filter(DBusConnection *connection, DBusMessage *msg, void *user_data)
{
	if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "Notify"))
	{
		m_data *data = (m_data*)user_data;
		
		DBusMessageIter iter;
		dbus_message_iter_init(msg, &iter);
		
		int type = dbus_message_iter_get_arg_type(&iter);
		
		char *val;
		for (int i = 0; i < 3; i++)
			dbus_message_iter_next (&iter);
		dbus_message_iter_get_basic (&iter, &val);
		
		char *notify_h, *notify_b;
		switch (data->lang)
		{
			case en:
				notify_h = NOTIFY_H_MSG_EN;
				notify_b = NOTIFY_B_MSG_EN;
			break;
			case ru:
				notify_h = NOTIFY_H_MSG_RU;
				notify_b = NOTIFY_B_MSG_RU;
			break;
			case de:
				notify_h = NOTIFY_H_MSG_DE;
				notify_b = NOTIFY_B_MSG_DE;
			break;
			case cs:
				notify_h = NOTIFY_H_MSG_CS;
				notify_b = NOTIFY_B_MSG_CS;
			break;
			case fr:
				notify_h = NOTIFY_H_MSG_FR;
				notify_b = NOTIFY_B_MSG_FR;
			break;
		}
		if (strcmp(val, notify_h) == 0) 
		{
			dbus_message_iter_next (&iter);
			dbus_message_iter_get_basic (&iter, &val);
			
			if (strcmp(val, notify_b) == 0)
			{
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

#define help() fputs(HELP, stdout)

int main(int argc, char **argv)
{
	/*if (argc <= 6)
	{
		help();
		return EXIT_FAILURE;
	}*/
	
	m_data data = {NULL, 540, 360, ru};

	const char *short_options = "x:y:X:Y:l:h";
	const struct option long_options[] =
	{
		{"lang", required_argument, NULL, 'l'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	
	int opt;
	int opt_index = -1;
	while ((opt = getopt_long(argc, argv, short_options, long_options, &opt_index)) != -1)
	{
		switch (opt)
		{
			case 'X': case 'x':
				sscanf(optarg, "%d", &data.x);
			break;
			case 'Y': case 'y':
				sscanf(optarg, "%d", &data.y);
			break;
			case 'l':
				if (strcmp(optarg, "en") == 0)
					data.lang = en;
				else if (strcmp(optarg, "ru") == 0)
					data.lang = ru;
				else if (strcmp(optarg, "de") == 0)
					data.lang = de;
				else if (strcmp(optarg, "cs") == 0)
					data.lang = cs;
				else if (strcmp(optarg, "fr") == 0)
					data.lang = fr;
				else
				{
					printf("Unknow lang!\n");
					return EXIT_FAILURE;
				}
			break;
			case 'h': case '?':
				help();
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
