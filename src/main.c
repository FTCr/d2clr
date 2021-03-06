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
//#include <X11/extensions/XTest.h>
#include <sys/stat.h>
#include <errno.h>

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

#define HELP "Usage: d2clrd [OPTIONS]...\n" \
"Actions:\n" \
"  -m                       Minimize after accept.\n" \
"  -h, --help               Print help.\n"

#define DEFAULT_SETTINGS {NULL, /*775, 425,*/ ru, FALSE}

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
	/*int x;
	int y;*/
	short lang;
	char is_min;
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
				system("xdotool windowactivate `xdotool search --name \"Dota 2\"`");
				sleep(1);
				system("xdotool key KP_Enter");
				/*
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
				}*/
				if (data->is_min == TRUE)
					system("xdotool windowminimize `xdotool search --name \"Dota 2\"`");
			}
		}
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}

short get_steam_lang()
{
	//getting steam config file path
	char *home = getenv("HOME");
	if (home == NULL) return -1;

	char *steam_cfg_path = malloc(strlen(home) + sizeof(char) * 20);
	if (steam_cfg_path == NULL) return -1;

	strcpy(steam_cfg_path, home);
	strcat(steam_cfg_path, "/.steam/registry.vdf");

	//read config file
	FILE *file = fopen(steam_cfg_path, "r");
	free(steam_cfg_path);
	if (file == NULL) return -1;
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buffer = malloc(size + 1);
	if (buffer == NULL)
	{
		fclose(file);
		return -1;
	}
	fread(buffer, size, sizeof(char), file);
	fclose(file);

	//find lang
	char lang[60];
	char *p = buffer, *p2 = NULL;
	while(*p != '\0')
	{
		if (strncmp(p, "Language", 8) == 0)
		{
			p += sizeof(char) * (8 + 1);
			p = strchr(p, '"') + sizeof(char);
			p2 = strchr(p, '"');
			*p2 = '\0';
			strcpy(lang, p);
		}

		p += sizeof(char);
	}
	free(buffer);
	if (strcmp(lang, "english") == 0) return en;
	else if (strcmp(lang, "russian") == 0) return ru;
	else if (strcmp(lang, "german") == 0) return de;
	else if (strcmp(lang, "czech") == 0) return cs;
	else if (strcmp(lang, "french") == 0) return fr;
	else return -1;
}

unsigned int p_exist(unsigned int pid)
{
	struct stat sts;
	char *filename = malloc(sizeof(char) * 50);
	sprintf(filename, "/proc/%d", pid);
	if (stat(filename, &sts) == -1 && errno == ENOENT)
	{
		free(filename);
		return FALSE;
	}
	else
	{
		free(filename);
		return TRUE;
	}
}

#define help() fputs(HELP, stdout)

int main(int argc, char **argv)
{
	m_data data = DEFAULT_SETTINGS;

	const char *short_options = "hm";
	const struct option long_options[] =
	{
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	int opt;
	int opt_index = -1;
	while ((opt = getopt_long(argc, argv, short_options, long_options, &opt_index)) != -1)
	{
		switch (opt)
		{
			/*case 'X': case 'x':
				sscanf(optarg, "%d", &data.x);
			break;
			case 'Y': case 'y':
				sscanf(optarg, "%d", &data.y);
			break;*/
			case 'm':
				data.is_min = TRUE;
			break;
			case 'h': case '?':
				help();
				return EXIT_FAILURE;
		}
	}
	data.display = XOpenDisplay(NULL);
	if (!data.display)
	{
		fprintf(stdout, "Cannot open display!\n");
		return EXIT_FAILURE;
	}

	if ((data.lang = get_steam_lang()) == -1)
	{
		fprintf(stdout, "Language not found!\n");
		return EXIT_FAILURE;
	}

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	DBusError error;
	dbus_error_init(&error);

	DBusConnection *connect = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (dbus_error_is_set(&error))
	{
		fprintf(stderr, "Cannot get System BUS Connection: %s\n", error.message);
		dbus_error_free(&error);
		return EXIT_FAILURE;
	}

	dbus_connection_setup_with_g_main(connect, NULL);

	dbus_bus_add_match(connect, DBUS_RULE, &error);

	if (dbus_error_is_set(&error))
	{
		fprintf(stderr, "Cannot add D-BUS match rule, cause: %s\n", error.message);
		dbus_error_free(&error);
		return EXIT_FAILURE;
	}

	dbus_connection_add_filter(connect, signal_filter, &data, NULL);

	pid_t pidf = fork();
	if (pidf == -1)
	{
		fprintf(stderr, "Cannot create daemon!\n");
		XCloseDisplay(data.display);
		return EXIT_FAILURE;
	}
	else if (!pidf)
	{
		setsid();
		fprintf(stdout, "Listening D-BUS...\n");
		g_main_loop_run(loop);
		return EXIT_SUCCESS;
	}
	else
	{
		const char *home = getenv("HOME");
		if (home == NULL) return EXIT_FAILURE;

		char *filename = malloc((strlen(home) + strlen("/.cache/d2clrd.pid")) * sizeof(char));
		if (filename == NULL) return EXIT_FAILURE;
		strcpy(filename, home);
		strcat(filename, "/.cache/d2clrd.pid");

		FILE *file = fopen(filename, "a+");
		if (file == NULL)
		{
			fprintf(stderr, "Cannot create or open PID file!\n");
			kill(pidf, SIGTERM);
			XCloseDisplay(data.display);
			return EXIT_FAILURE;
		}

		pid_t pid = 0;
		char *buffer = malloc(sizeof(char) * 10);
		fread(buffer, 10, sizeof(char), file);
		fclose(file);
		pid = strtoul(buffer, NULL, 10);
		free(buffer);

		if (pid)
		{
			if (pid != pidf)
			{
				if (p_exist(pid))
				{
					fprintf(stderr, "d2clrd already running!\n");
					kill(pidf, SIGTERM);
					XCloseDisplay(data.display);
					return EXIT_FAILURE;
				}
				else
				{
					pid = pidf;
				}
			}
		}
		else
		{
			pid = pidf;
		}
		file = fopen(filename, "w");
		free(filename);
		fprintf(file, "%d", pid);
		fclose(file);
		return EXIT_SUCCESS;
	}
}
