#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { fprintf(log_file, MSG); fprintf(log_file, "\n"); exit(-1); }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { fprintf(log_file, MSG); fprintf(log_file, "\n"); exit(-1); }
const char* TEST_OUTPUT_FILE = "./logs/log.txt";
const int COUNT_SECTIONS_MAIN_MENU = 2;
const int COUNT_SECTIONS_NEW_GAME = 4;
const char* MENU_SECTION[COUNT_SECTIONS_MAIN_MENU] = { "New game", "Exit" };
const char* NEW_GAME_SECTION[COUNT_SECTIONS_NEW_GAME] = { "New game with 1 player", "New game with 2 players", "Connect to existing game", "Back to main menu" };
enum Menu {
	NEW_GAME = 0,
	EXIT = 1,
	NEW_GAME_1_PLAYER = 2,
	NEW_GAME_2_PLAYERS = 3,
	CONNECT_TO_EXIST_GAME = 4, 
	BACK_TO_MAIN_MENU = 5
};
const int WINDOW_SIZE_LINES = 25;
const int WINDOW_SIZE_COLUMNS = 90;
const int WINDOW_LEFT_UP_X = 0;
const int WINDOW_LEFT_UP_Y = 0;
const int CENTER_WINDOW_Y = (WINDOW_SIZE_LINES - WINDOW_LEFT_UP_Y) / 2 - 2;
const int CENTER_WINDOW_X = (WINDOW_SIZE_COLUMNS - WINDOW_LEFT_UP_X) / 2;

const char* NAME_PLAYER_EXECUTION = "player";
const int PORT_SIZE = 5;

bool ReadPort(WINDOW* window, std::string& port) {
	box(window, 0, 0);
	wclear(window);
	mvwprintw(window, CENTER_WINDOW_Y+1, CENTER_WINDOW_X-10, "Input port game: ");
	echo();
	wrefresh(window);
	int input = 1;
	int index = 0;
	while (input) {
		char c = wgetch(window);
		if (c == 10) {
			break;
		}
		else if (c == 27) {
			input = 0;
			break;
		}
		else if (0 <= c - '0' && c - '0' <= 9 && index < PORT_SIZE) {
			port.push_back(c);
		}
	}
	noecho();
	wclear(window);
	if (!input) {
		return false;
	}
	return true;
}

int main() {
	FILE* log_file = NULL;
	log_file = fopen(TEST_OUTPUT_FILE, "w");
	if (log_file == NULL) {
		printf("Error creating log file!\n");
		return -1;
	}
	
	WINDOW* terminal = initscr();
	check_wrong(terminal, NULL, "Error initialising ncurses!");
	check_ok(start_color(), OK, "Error initialising colors mode!");
	WINDOW* menu = newwin(WINDOW_SIZE_LINES, WINDOW_SIZE_COLUMNS, WINDOW_LEFT_UP_X, WINDOW_LEFT_UP_Y);
	check_wrong(menu, NULL, "Error creating new window!");
	if (has_colors() && COLOR_PAIRS >= 13) {
		check_ok(init_pair(1, COLOR_BLACK, COLOR_WHITE), OK, "Error creating new color pair!");
	}
	else {
		fprintf(log_file, "Error working colors mode!");
		return -1;
	}
	check_ok(raw(), OK, "Error changing raw mode of working!");
	check_ok(wbkgd(menu, COLOR_PAIR(1)), OK, "Error changing background color of window!");
	check_ok(keypad(menu, TRUE), OK, "Error of enabling mode processing special keys!");
	check_ok(noecho(), OK, "Error make noecho mode of working!");
	curs_set(0);
	
	int option_now = 0;
	int button;
	int type = 0;
	while (TRUE) {
		box(menu, 0, 0);
		for (int i = 0; i < (type == 0? COUNT_SECTIONS_MAIN_MENU : COUNT_SECTIONS_NEW_GAME); ++i) {
			if (i == option_now) {
				check_ok(wattron(menu, A_BLINK | A_BOLD), OK, "Error changing window attribute!");
			}
			check_ok(mvwprintw(menu, CENTER_WINDOW_Y + i*2, CENTER_WINDOW_X - 10, (type == 0? MENU_SECTION[i]: NEW_GAME_SECTION[i])), OK, "Error output menu info!");
			check_ok(wattroff(menu, A_BLINK | A_BOLD), OK, "Error changing window attribute!");
		}
		wrefresh(menu);
		button = wgetch(menu);
		check_wrong(button, ERR, "Error input button in program!");
		fprintf(log_file, "Getting key -> %d\n", button);
		if (button == 27) {
			if (type == 0) {
				break;
			}
			else {
				type = 0;
			}
		}
		else if (button == 10) {
			if (option_now+type == EXIT) {
				break;
			}
			else if (option_now+type == NEW_GAME) {
				type = 2;
				option_now = 0;
			}
			else if (option_now+type == NEW_GAME_1_PLAYER) {
				delwin(menu);
				endwin();
				fclose(log_file);
				check_wrong(execl(NAME_PLAYER_EXECUTION, NAME_PLAYER_EXECUTION, std::to_string(0).c_str(), std::to_string(2).c_str(), NULL), -1, "Error changing execution program!");
			}
			else if (option_now+type == NEW_GAME_2_PLAYERS) {
				std::string port;
				if (ReadPort(menu, port)) {
					delwin(menu);
					endwin();
					fclose(log_file);
					check_wrong(execl(NAME_PLAYER_EXECUTION, NAME_PLAYER_EXECUTION, port.c_str(), std::to_string(0).c_str(), NULL), -1, "Error changing execution program!");
				}
			}
			else if (option_now+type == CONNECT_TO_EXIST_GAME) {
				std::string port;
				if (ReadPort(menu, port)) {
					delwin(menu);
					endwin();
					fclose(log_file);
					check_wrong(execl(NAME_PLAYER_EXECUTION, NAME_PLAYER_EXECUTION, port.c_str(), std::to_string(1).c_str(), NULL), -1, "Error changing execution program!");
				}
			}
			else if (option_now+type == BACK_TO_MAIN_MENU) {
				option_now = 0;
				type = 0;
			}
		}
		else if (button == KEY_UP) {
			--option_now;
			if (option_now < 0) {
				option_now = (type == 0? COUNT_SECTIONS_MAIN_MENU : COUNT_SECTIONS_NEW_GAME) - 1;
			}
		}
		else if (button == KEY_DOWN) {
			++option_now;
			if (option_now == (type == 0? COUNT_SECTIONS_MAIN_MENU : COUNT_SECTIONS_NEW_GAME)) {
				option_now = 0;
			}
		}
		wclear(menu);
	}
	
	check_ok(endwin(), OK, "Error closing ncurses!");
	fprintf(log_file, "Exiting from program!");
	fclose(log_file);
}
