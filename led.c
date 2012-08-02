
/* led - a lightweight Line EDitor for Linux(R) and Lanos.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 *
 * Contributers:
 * 		eXerigumo Clanjor <cjxgm@126.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constants
#define MAX_LINE					9999
#define MAX_CH_PER_LINE				256
/* terminal */
#define COLOR_NORMAL				"\e[m"
#define COLOR_LOGO					"\e[33m"
#define COLOR_BLANK_LINE			"\e[34m"
#define COLOR_LINE					"\e[m"
#define COLOR_LINENO				"\e[33m"
#define COLOR_LINE_TOO_LONG			"\e[m"
#define COLOR_CURRENT_LINE			"\e[1;37;44m"
#define COLOR_CURRENT_LINENO		"\e[1;33;40m"
#define COLOR_CURRENT_LINE_TO_LONG	"\e[m"
#define COLOR_STATUS_INFO			"\e[1m"
#define COLOR_STATUS_ERROR			"\e[1;31m"
#define COLOR_CONFIRM				"\e[1m"
/* ui */
#define MAX_TMP_STRING				(MAX_CH_PER_LINE-1)
#define STATUS_INFO					0
#define STATUS_ERROR				1


// global variables
static unsigned char text[MAX_LINE][MAX_CH_PER_LINE];
static int used_line_cnt;
static int current_line;
/* ui */
static const char * status = "";
static int status_type = STATUS_INFO;
static char tmp_string[MAX_TMP_STRING+1];


// function declares
/* terminal */
static void term_clear();	// clear screen and reset cursor to upper-left
static void term_cbne();	//  enable cbreak, set no echo
static void term_nocbne();	// disable creabk, set to echo
/* ui */
static void print_line(int lineno);
static void print_status();
static void read_string();
static int  confirm(const char * msg);
/* commands */
static void cmd_create();
static void cmd_append();
static void cmd_delete();
static void cmd_prev();
static void cmd_next();
static void cmd_backspace();
static void cmd_newline();


/**********************************************************************
 *
 * main
 *
 */

int main()
{
	term_cbne();
	term_clear();

	// show logo
	printf(
		COLOR_LOGO
		"                                                            \n"
		"           .   .--   .                                      \n"
		"           |   |-- .-|  a lightweight line editor           \n"
		"           '-- '-- '-'                                      \n"
		"                                                            \n"
		COLOR_NORMAL
		"                press any key to start...                   \n"
	);
	getchar();

	// init
	cmd_create();

	// command loop
	while (1) {
		term_clear();
		print_line(current_line - 2);
		print_line(current_line - 1);
		print_line(current_line);
		print_line(current_line + 1);
		print_line(current_line + 2);
		print_status();
		status = "";
		status_type = STATUS_INFO;

		int ch = getchar();
		switch (ch) {
			case 'c':
				if (confirm("any unsaved data will be lost, sure?"))
					cmd_create();
				break;
			case 'q':
				if (confirm("any unsaved data will be lost, sure?"))
					goto _exit_command_loop;
				break;
			case 'a':	cmd_append();		break;
			case 'd':	cmd_delete();		break;
			case 'p':	cmd_prev();			break;
			case 'n':	cmd_next();			break;
			case '\b':	cmd_backspace();	break;
			case '\n':	cmd_newline();		break;
			default:
				status = "unknown command";
				status_type = STATUS_ERROR;
				break;
		}
	}
_exit_command_loop:

	term_nocbne();
	printf(COLOR_NORMAL "\n");
	return 0;
}

/**********************************************************************
 *
 * terminal
 *
 */

static void term_clear()
{
	printf("\e[H\e[J");
}

static void term_cbne()
{
	system("stty cbreak -echo");
}

static void term_nocbne()
{
	system("stty -cbreak echo");
}

/**********************************************************************
 *
 * ui
 *
 */

static void print_line(int lineno)
{
	if (lineno < 0 || lineno >= used_line_cnt) {
		printf(COLOR_BLANK_LINE "~~~~\n" COLOR_NORMAL);
		return;
	}

	if (lineno == current_line) printf(COLOR_CURRENT_LINENO);
	else printf(COLOR_LINENO);
	printf("%04d ", lineno+1);
	if (lineno == current_line) printf(COLOR_CURRENT_LINE);
	else printf(COLOR_LINE);
	printf("%-74s\n" COLOR_NORMAL, text[lineno]);
}

static void print_status()
{
	switch (status_type) {
		case STATUS_INFO:
			printf(COLOR_STATUS_INFO " %s\n" COLOR_NORMAL, status);
			break;
		case STATUS_ERROR:
			printf(COLOR_STATUS_ERROR " %s\n" COLOR_NORMAL, status);
			break;
	}
}

static void read_string()
{
	term_nocbne();
	printf(":");
	if (fgets(tmp_string, MAX_TMP_STRING, stdin) != tmp_string) {
		status = "unable to read from stdin";
		status_type = STATUS_ERROR;
	}
	int last = strlen(tmp_string)-1;
	if (last>=0 && tmp_string[last] == '\n')
		tmp_string[last] = 0;
	term_cbne();
}

static int confirm(const char * msg)
{
	printf(COLOR_CONFIRM "%s (y/n) "COLOR_NORMAL, msg);
	return getchar() == 'y';
}

/**********************************************************************
 *
 * commands
 *
 */

static void cmd_create()
{
	int i;
	for (i=0; i<MAX_LINE; i++)
		text[i][0] = 0;
	used_line_cnt = 1;
	current_line  = 0;
}

static void cmd_append()
{
	int len = strlen(text[current_line]);
	if (len == MAX_CH_PER_LINE-1) {
		status = "line is too long!";
		status_type = STATUS_ERROR;
	}

	read_string();
	if (len + strlen(tmp_string) > MAX_CH_PER_LINE-1) {
		status = "line is too long!";
		status_type = STATUS_ERROR;
	}

	strncat(text[current_line], tmp_string, MAX_CH_PER_LINE-1-len);
}

static void cmd_delete()
{
	text[current_line][0] = 0;
	if (used_line_cnt == 1) return;

	used_line_cnt--;
	int i;
	for (i=current_line; i<used_line_cnt; i++)
		strcpy(text[i], text[i+1]);
	text[used_line_cnt][0] = 0;
	if (current_line == used_line_cnt)
		current_line--;
}

static void cmd_prev()
{
	if (--current_line < 0)
		current_line = 0;
}

static void cmd_next()
{
	if (++current_line == used_line_cnt)
		current_line--;
}

static void cmd_backspace()
{
	int len = strlen(text[current_line]);
	if (len) text[current_line][len-1] = 0;
}

static void cmd_newline()
{
	used_line_cnt++;
	current_line++;

	// no need to move data
	if (current_line == used_line_cnt-1)
		return;

	// move data
	int i;
	for (i=used_line_cnt-1; i>current_line; i--)
		strcpy(text[i], text[i-1]);
	text[current_line][0] = 0;
}

// vim: ts=4 sw=4 sts=0 noet fenc=utf-8

