
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
#define COLOR_STATUS_INFO			"\e[1;37;40m"
#define COLOR_STATUS_ERROR			"\e[1;31m"
#define COLOR_CONFIRM				"\e[1;31m"
#define COLOR_RULER					"\e[1;32m"
/* ui */
#define MAX_TMP_STRING				(MAX_CH_PER_LINE-1)
#define STATUS_INFO					0
#define STATUS_ERROR				1


// global variables
static unsigned char text[MAX_LINE][MAX_CH_PER_LINE];
static int used_line_cnt;
static int current_line;
static int find_line;
static int find_direction;
static char find_keyword[MAX_CH_PER_LINE];
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
static void print_ruler();
static int  read_integer();
static void read_string();
static void strip_tmp_string();
static int  confirm(const char * msg);
/* commands */
static void cmd_create();
static void cmd_append();
static void cmd_insert();
static void cmd_delete(int cnt);
static void cmd_prev(int cnt);
static void cmd_next(int cnt);
static void cmd_save();
static void cmd_load();
static void cmd_head();
static void cmd_tail();
static void cmd_view(int maxline);
static void cmd_gotoline();
static void cmd_backspace();
static void cmd_newline();
static void cmd_find(int direction);
static void cmd_findnext();
static void cmd_help();


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
			case 'a':	cmd_append();						break;
			case 'i':	cmd_insert();						break;
			case 'd':	cmd_delete(1);						break;
			case 'D':	cmd_delete(read_integer());			break;
			case 'p':	cmd_prev(1);						break;
			case 'P':	cmd_prev(read_integer());			break;
			case 'n':	cmd_next(1);						break;
			case 'N':	cmd_next(read_integer());			break;
			case 's':	cmd_save();							break;
			case 'l':	cmd_load();							break;
			case 'h':	cmd_head();							break;
			case 't':	cmd_tail();							break;
			case 'v':	cmd_view(20);						break;
			case 'V':	cmd_view(read_integer());			break;
			case '>':	cmd_gotoline();						break;
			case 'f':	cmd_find(+1);						break;
			case 'F':	cmd_find(-1);						break;
			case ' ':	cmd_findnext();						break;
			case '?':	cmd_help();							break;
			case 127:	cmd_backspace();					break;
			case '\n':	cmd_newline();						break;
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
#ifdef __LANOS__
	clear_screen();
#else
	printf("\e[H\e[J");
#endif
}


static void term_cbne()
{
#ifndef __LANOS__
	system("stty cbreak -echo");
#endif
}


static void term_nocbne()
{
#ifndef __LANOS__
	system("stty -cbreak echo");
#endif
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


static void print_ruler(int length)
{
	int i;

	printf(COLOR_RULER "     ");
	for (i=0; i<length; i++)
		printf("%d", i/10);
	printf("\n");

	printf("     ");
	for (i=0; i<length; i++)
		printf("%d", i%10);
	printf("\n" COLOR_NORMAL);
}


static int read_integer()
{
	term_nocbne();
	printf(">");
	if (fgets(tmp_string, MAX_TMP_STRING, stdin) != tmp_string) {
		status = "unable to read from stdin";
		status_type = STATUS_ERROR;
		term_cbne();
		return 0;
	}
	term_cbne();
	return atoi(tmp_string);
}


static void read_string()
{
	term_nocbne();
	printf(":");
	if (fgets(tmp_string, MAX_TMP_STRING, stdin) != tmp_string) {
		status = "unable to read from stdin";
		status_type = STATUS_ERROR;
	}
	strip_tmp_string();
	term_cbne();
}


static int confirm(const char * msg)
{
	printf(COLOR_CONFIRM "%s (y/n) "COLOR_NORMAL, msg);
	return getchar() == 'y';
}


static void strip_tmp_string()
{
	int i;
	int len = strlen(tmp_string);
	// strip tab
	for (i=0; i<len; i++)
		if (tmp_string[i] == '\t')
			tmp_string[i] = ' ';
	// strip the last '\n'
	if (len>0 && tmp_string[len-1] == '\n')
		tmp_string[len-1] = 0;
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


static void cmd_insert()
{
	int i, j;

	if (strlen(text[current_line]) > 74) {
		status = "line is too long";
		status_type = STATUS_ERROR;
		return;
	}
	if (!text[current_line][0]) {
		status = "nothing to do with this";
		return;
	}

	term_clear();
	print_ruler(strlen(text[current_line]));
	print_line(current_line);
	printf("\n\n");

	int pos = read_integer();
	if (pos < 0 || pos > 73) {
		status = "nothing to do with this";
		return;
	}

	term_clear();
	print_ruler(strlen(text[current_line]));
	print_line(current_line);
	printf("     ");
	for (i=0; i<pos; i++) printf(" ");
	printf(COLOR_RULER "^" COLOR_NORMAL);
	printf("\n\n");

	read_string();
	if (!tmp_string[0]) return;

	int inslen = strlen(tmp_string);
	j = strlen(text[current_line]);
	for (i=j+inslen; j>=pos; i--,j--)
		text[current_line][i] = text[current_line][j];
	for (i=0; i<inslen; i++)
		text[current_line][pos+i] = tmp_string[i];
}


static void cmd_delete(int cnt)
{
	if (cnt <= 0) {
		status = "nothing to do with this";
		return;
	}
	while (cnt--) {
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
}


static void cmd_prev(int cnt)
{
	if (cnt <= 0) {
		status = "nothing to do with this";
		return;
	}
	if ((current_line -= cnt) < 0)
		current_line = 0;
}


static void cmd_next(int cnt)
{
	if (cnt <= 0) {
		status = "nothing to do with this";
		return;
	}
	if ((current_line += cnt) >= used_line_cnt)
		current_line = used_line_cnt - 1;
}


static void cmd_save()
{
	read_string();
	if (!strlen(tmp_string)) {
		status = "canceled";
		return;
	}

	FILE * fp = fopen(tmp_string, "w");
	if (!fp) {
		status = "file open failed!";
		status_type = STATUS_ERROR;
		return;
	}

	int i;
	for (i=0; i<used_line_cnt; i++)
		fprintf(fp, "%s\n", text[i]);
	fclose(fp);
	status = "saved";
}


static void cmd_load()
{
	read_string();
	if (!strlen(tmp_string)) {
		status = "canceled";
		return;
	}

	FILE * fp = fopen(tmp_string, "r");
	if (!fp) {
		status = "file open failed!";
		status_type = STATUS_ERROR;
		return;
	}

	cmd_create();
	while (fgets(tmp_string, MAX_TMP_STRING, fp) == tmp_string) {
		if (used_line_cnt == MAX_LINE) {
			status = "too many lines!";
			status_type = STATUS_ERROR;
			fclose(fp);
			cmd_delete(1);
			status = "file loading stopped half way";
			return;
		}
		strip_tmp_string();
		strcpy(text[used_line_cnt++], tmp_string);
	}
	fclose(fp);
	cmd_delete(1);
	status = "loaded";
}


static void cmd_head()
{
	current_line = 0;
}


static void cmd_tail()
{
	current_line = used_line_cnt - 1;
}


static void cmd_view(int maxline)
{
	if (maxline <= 0) {
		status = "nothing to do with this";
		return;
	}

	int backup = current_line;
	int i;

	current_line = -1;

	for (i=0; i<used_line_cnt; i++) {
		if (i % maxline == 0) {
			term_clear();
			printf(COLOR_STATUS_INFO "view %04d/%04d\n" COLOR_NORMAL,
					i+1, used_line_cnt);
		}
		print_line(i);
		if (i % maxline == maxline - 1) {
			printf(COLOR_STATUS_INFO
					"press any key to continue..." COLOR_NORMAL);
			getchar();
		}
	}
	if (used_line_cnt % maxline) {
		printf(COLOR_STATUS_INFO
				"press any key to continue..." COLOR_NORMAL);
		getchar();
	}

	current_line = backup;
}


static void cmd_gotoline()
{
	int lineno = read_integer();
	if (lineno <= 0) {
		status = "nothing to do with this";
		return;
	}
	if (lineno > used_line_cnt) {
		status = "no such line";
		status_type = STATUS_ERROR;
		return;
	}
	current_line = lineno - 1;
}


static void cmd_backspace()
{
	int len = strlen(text[current_line]);
	if (len) text[current_line][len-1] = 0;
}


static void cmd_newline()
{
	if (used_line_cnt == MAX_LINE) {
		status = "too many lines!";
		status_type = STATUS_ERROR;
		return;
	}
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


static void cmd_find(int direction)
{
	read_string();
	if (!tmp_string[0]) {
		if (find_keyword[0]) {
			find_direction = direction;
			cmd_findnext();
		}
		return;
	}

	strcpy(find_keyword, tmp_string);
	find_direction = direction;
	find_line = current_line;
	cmd_findnext();
}


static void cmd_findnext()
{
	if (find_direction != 1 && find_direction != -1) {
		status = "nothing to do with this";
		return;
	}

	int i;
	if (find_direction == 1) {
		for (i=find_line; i<used_line_cnt; i++) {
			if (strstr(text[i], find_keyword)) {
				current_line = i;
				find_line    = i + 1;
				return;
			}
		}
	}
	else {
		for (i=find_line; i>=0; i--) {
			if (strstr(text[i], find_keyword)) {
				current_line = i;
				find_line    = i - 1;
				return;
			}
		}
	}

	// not found
	status = "nothing found";
	status_type = STATUS_ERROR;
}


static void cmd_help()
{
	term_clear();
	printf(
		COLOR_LOGO
		"                                                            \n"
		"           .   .--   .                                      \n"
		"           |   |-- .-|  a lightweight line editor           \n"
		"           '-- '-- '-'                                      \n"
		"                                                            \n"
		COLOR_STATUS_INFO
		"                         H E L P                            \n"
		COLOR_LINENO
		"   c create   a append    n next line    f find forward     \n"
		"   l load     i insert    N next line*   F find backward    \n"
		"   s save     d delete    p prev line    v view all         \n"
		"   q quit     D delete*   P prev line*   V view all*        \n"
		"   h head     t tail      > go to line   ? help (this)      \n"
		"   <backspace> remove last character of current line.       \n"
		"   <return>    new line next to current line.               \n"
		"   <space>     find next.                                   \n"
		COLOR_STATUS_ERROR
		"         note: '*' means it will ask you a count.           \n"
		"                                                            \n"
		COLOR_STATUS_INFO
		"                 C O N T R I B U T E R S                    \n"
		COLOR_LINENO
		"             eXerigumo Clanjor <cjxgm@126.com>              \n"
		"                                                            \n"
		COLOR_NORMAL
		"                press any key to go back..."
	);
	getchar();
}


// vim: ts=4 sw=4 sts=0 noet fenc=utf-8

