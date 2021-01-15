#include "interface.hpp"
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { fprintf(log_file, MSG); fprintf(log_file, "\n"); exit(-1); }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { fprintf(log_file, MSG); fprintf(log_file, "\n"); exit(-1); }
const char* LOG_FILE = "./logs/plog.txt";

const int WINDOW_SIZE_LINES = 25;
const int WINDOW_SIZE_COLUMNS = 90;
const int WINDOW_LEFT_UP_X = 0;
const int WINDOW_LEFT_UP_Y = 0;
const int CENTER_WINDOW_Y = (WINDOW_SIZE_LINES - WINDOW_LEFT_UP_Y) / 2;
const int CENTER_WINDOW_X = (WINDOW_SIZE_COLUMNS - WINDOW_LEFT_UP_X) / 2;

int PLAYER_POS = 1;
const int CONNECT_PLAYER_POS = 12;
const int PLAYER_POS_LINES = 15;
const int GROUND_LINES = 20;
const int SPACE_STRENGTH = 10;

const int GAME_SPEED = 50000;
const int GAME_CHANGE_SPEED_DELAY = 10;
const int GAME_CHANGE_SPEED_VALUE = 1000;

const int SCORE_DIGITS = 5;
const int SCORE_DELAY = 10;

const int TREES_TYPES_COUNT = 3;
const int TEXTURE_HEIGHT = 5;
const char* TREES[TREES_TYPES_COUNT][TEXTURE_HEIGHT] = {{ "   Y   ", "Y  Y  Y", " YYYYY ", "   Y   ", "   Y   " }, 
{" ", " ", "Y  Y  Y", " YYYYY ", "   Y   "}, {" ", " ", "  Y Y Y", "   YYY", "    Y" }};
const int MAX_COUNT_TREES = 10;
const int TREES_DISTANCE = 10;
const int TREE_PROPABILITY = 10;
const int TREE_SUCCESS = 9;

const char* MAIN_MENU_PROGRAM_NAME = "game";

void DinoDraw(WINDOW* window, int x, int y, int bend_down, int type) {
	static int step1 = 0;
	static int step2 = 1;
	wattron(window, COLOR_PAIR(type));
	if (bend_down == 0) {
		mvwprintw(window, x, y,   "       ^");
		mvwprintw(window, x+1, y, "I     OO=");
		mvwprintw(window, x+2, y, "I OOOOOO");
		mvwprintw(window, x+3, y, " OOOOOO");
		if (x == PLAYER_POS_LINES) {
			mvwprintw(window, x+4, y, "  O  O");
		}
	}
	else {
		mvwprintw(window, x+2, y, "        ^");
		mvwprintw(window, x+3, y, "I OOOOOOO=");
		mvwprintw(window, x+4, y, "I OOOOOO");
	}
	if (x == PLAYER_POS_LINES) {
		if ((type == 1 && step1 == 0) || (type == 2 && step2 == 0)) {
			mvwprintw(window, x+5, y, "  __ ==");
		}
		else {
			mvwprintw(window, x+5, y, "  == __");
		}
		if (type == 1) {
			step1 = (step1 == 0? 1 : 0);
		}
		else if (type == 2) {
			step2 = (step2 == 0? 1 : 0);
		}
	}
	else {
		mvwprintw(window, x+4, y, "  == ==");
	}
	wattroff(window, COLOR_PAIR(type));
}
void TreeDraw(WINDOW* window, int tree_num, int x, int y, int* interact) {
	for (int i = 0; i < TEXTURE_HEIGHT; ++i) {
		for (int j = 0; TREES[tree_num][i][j] != '\0'; ++j) {
			if (0 <= x+i &&  x+i < WINDOW_SIZE_LINES && 0 <= y+j && y+j < WINDOW_SIZE_COLUMNS) {
				char c = mvwinch(window, x+i, y+j) & A_CHARTEXT;
				if (TREES[tree_num][i][j] != ' ') {
					if (TREES[tree_num][i][j] == 'Y') {
						int color = mvwinch(window, x+i, y+j) & A_COLOR;
						if (color == COLOR_PAIR(1)) {
							if (c == 'O' || c == '=' || c == '^' || c == '_') {
								*interact = 1;
							}
						}
					}
					mvwaddch(window, x+i, y+j, TREES[tree_num][i][j]);
				}
			}
		}
	}
}
void GroundDraw(WINDOW* window) {
	static int mode = 0;
	mode = (mode == 0? 1 : 0);
	for (int i = 0; i < WINDOW_SIZE_COLUMNS; ++i) {
		if (mode) {
			mvwprintw(window, GROUND_LINES, i, (i % 2 == 0? "-" : "~"));	
		}
		else {
			mvwprintw(window, GROUND_LINES, i, (i % 2 == 0? "~" : "-"));
		}
	}
}
void ScoreDraw(WINDOW* window, int score1, int score2) {
	char answer1[SCORE_DIGITS+1];
	char answer2[SCORE_DIGITS+1];
	for (int i = SCORE_DIGITS-1; i >= 0; --i) {
		answer1[i] = '0' + score1 % 10;
		answer2[i] = '0' + score2 % 10;
		score1 /= 10;
		score2 /= 10;
	}
	answer1[SCORE_DIGITS] = '\0';
	answer2[SCORE_DIGITS] = '\0';
	mvwprintw(window, 2, 40, "YOUR SCORE: %s\tENEMY SCORE: %s", answer1, answer2);
}
int main(int argc, char* argv[]) {
	int res;
	srand(time(0));
	FILE* log_file = fopen(LOG_FILE, "w");
	check_wrong(log_file, NULL, "Error open log file!");
	check_ok(argc, 3, "Incorrect count arguments in child process!");
	int port = atoi(argv[1]);
	int mode = atoi(argv[2]);
	
	void* my_context, *my_socket;
	
	if (mode != 2) {
		char address[104];
		CreateSocket(my_context, my_socket);
		if (mode == 0) {
			snprintf(address, sizeof(address), "tcp://*:%d", port);
			res = zmq_bind(my_socket, address);
			check_ok(res, 0, "Error creating bind connection!");
			fprintf(log_file, "%s\n", address);
		}
		else if (mode == 1) {
			snprintf(address, sizeof(address), "tcp://localhost:%d", port);
			res = zmq_connect(my_socket, address);
			check_ok(res, 0, "Error creating outgoing connection!");
			fprintf(log_file, "%s\n", address);
		}
	}

	WINDOW* terminal = initscr();
	check_wrong(terminal, NULL, "Error initialising ncurses!");
	check_ok(start_color(), OK, "Error initialising colors mode!");
	WINDOW* gamewin = newwin(WINDOW_SIZE_LINES, WINDOW_SIZE_COLUMNS, WINDOW_LEFT_UP_X, WINDOW_LEFT_UP_Y);
	check_wrong(gamewin, NULL, "Error creating new window!");
	if (has_colors() && COLOR_PAIRS >= 13) {
		check_ok(init_pair(1, COLOR_BLACK, COLOR_WHITE), OK, "Error creating new color pair!");
		check_ok(init_pair(2, COLOR_RED, COLOR_WHITE), OK, "Error creating new color pair!");
	}
	else {
		fprintf(log_file, "Error working colors mode!");
		return -1;
	}
	check_ok(raw(), OK, "Error changing raw mode of working!");
	check_ok(wbkgd(gamewin, COLOR_PAIR(1)), OK, "Error changing background color of window!");
	check_ok(keypad(gamewin, TRUE), OK, "Error of enabling mode processing special keys!");
	check_ok(noecho(), OK, "Error make noecho mode of working!");
	nodelay(gamewin, TRUE);
	curs_set(0);
	
	Token reply;
	Token* request = new Token;
	if (mode == 0) {
		RecieveMessage(reply, my_socket, 0);
		if (reply.action == start) {
			request->action = start;
			request->data[0] = CONNECT_PLAYER_POS;
			SendMessage(request, my_socket, ZMQ_DONTWAIT);
		}
	}
	else if (mode == 1) {
		request->action = start;
		DialogMessages(request, reply, my_socket);
		if (reply.action == start) {
			PLAYER_POS = reply.data[0];
		}
	}
	int button;
	
	int bend_down = 0;
	int space_vers = 0;
	int player_pos = PLAYER_POS_LINES;
	
	int my_score = 0;
	int enemy_score = 0;
	int delay = 0;
	
	int trees[MAX_COUNT_TREES];
	int trees_pos[MAX_COUNT_TREES];
	
	int interact = 0;
	int enemy_lose = 0;
	int game_speed = GAME_SPEED;
	for (int i = 0; i < MAX_COUNT_TREES; ++i) {
		trees[i] = -1;
	}
	
	if (mode == 1) {
		delete request;
		request = new Token;
		request->action = info;
		request->data[0] = player_pos;
		request->data[1] = PLAYER_POS;
		request->data[2] = bend_down;
		request->data[3] = -1;
		request->data[4] = 1;
		SendMessage(request, my_socket, ZMQ_DONTWAIT);
	}
	reply.action = info;
	reply.data[0] = player_pos;
	reply.data[1] = (mode == 0? CONNECT_PLAYER_POS : 1);
	reply.data[2] = 0;
	reply.data[3] = -1;
	reply.data[4] = 1;
	
	if (mode == 2) {
		enemy_lose = 1;
	}
	while (!interact) {
		if (!enemy_lose) {
			Token new_data;
			if (RecieveMessage(new_data, my_socket, ZMQ_DONTWAIT) && new_data.action == info) {
				reply = new_data;
			}
		}
		usleep(game_speed);
		GroundDraw(gamewin);
		ScoreDraw(gamewin, my_score, enemy_score);
		if (space_vers != 0) {
			--space_vers;
			--player_pos;
		}
		else if (player_pos != PLAYER_POS_LINES) {
			++player_pos;
		}
		
		if (bend_down != 0) {
			--bend_down;
		}
		DinoDraw(gamewin, player_pos, PLAYER_POS, bend_down, 1);
		if (!enemy_lose) {
			DinoDraw(gamewin, reply.data[0], reply.data[1], reply.data[2], 2);
		}
		for (int i = 0; i < MAX_COUNT_TREES; ++i) {
			if (trees[i] != -1) {
				TreeDraw(gamewin, trees[i], GROUND_LINES-TEXTURE_HEIGHT+1, trees_pos[i], &interact);
				trees_pos[i] -= 1;
				if (trees_pos[i] <= -10) {
					trees[i] = -1;
				}
			}
		}
		
		wrefresh(gamewin);
		
		button = wgetch(gamewin);
		if (button == 10) {
			break;
		}
		else if (button == ' ') {
			if (player_pos == PLAYER_POS_LINES && space_vers == 0 && bend_down == 0) {
				space_vers = SPACE_STRENGTH;
			}
		}
		else if (button == KEY_DOWN) {
			if (player_pos == PLAYER_POS_LINES && space_vers == 0) {
				bend_down = 7;
			}
		}
		
		++delay;
		if (delay % SCORE_DELAY == 0) {
			++my_score;
			if (my_score % GAME_CHANGE_SPEED_DELAY == 0) {
				game_speed += GAME_CHANGE_SPEED_VALUE;
			}
			if (!enemy_lose) {
				++enemy_score;
			}
			delay = 0;
		}
		
		if (!enemy_lose) {
			delete request;
			request = new Token;
			request->action = info;
			request->data[0] = player_pos;
			request->data[1] = PLAYER_POS;
			request->data[2] = bend_down;
			request->data[3] = -1;
			if (interact) {
				request->data[4] = -1;
			}
			else {
				request->data[4] = 1;
			}
		}
		if (mode == 0 || enemy_lose) {
			int create_tree = rand() % TREE_PROPABILITY;
			if (create_tree >= TREE_SUCCESS) {
				create_tree = rand() % TREES_TYPES_COUNT;
				if (!enemy_lose) {
					request->data[3] = create_tree;
				}
				for (int i = 0; i < MAX_COUNT_TREES; ++i) {
					if (trees[i] == -1) {
						trees[i] = create_tree;
						trees_pos[i] = WINDOW_SIZE_COLUMNS + 10;
					}
				}
			}
		}
		else {
			if (reply.data[3] != -1) {
				for (int i = 0; i < MAX_COUNT_TREES; ++i) {
					if (trees[i] == -1) {
						trees[i] = reply.data[3];
						trees_pos[i] = WINDOW_SIZE_COLUMNS + 10;
					}
				}
			}
		}
		
		if (reply.data[4] == -1) {
			enemy_lose = 1;
		}
		
		if (!enemy_lose) {
			SendMessage(request, my_socket, ZMQ_DONTWAIT);
		}
		
		wclear(gamewin);
	}
	if (interact) {
		nodelay(gamewin, FALSE);
		check_ok(wattron(gamewin, A_BLINK), OK, "Error changing window attribute!");
		check_ok(mvwprintw(gamewin, CENTER_WINDOW_Y-2, CENTER_WINDOW_X-4, "GAME OVER!"), OK, "Error output text!");
		check_ok(wattroff(gamewin, A_BLINK), OK, "Error changing window attribute!");
		check_ok(mvwprintw(gamewin, CENTER_WINDOW_Y, CENTER_WINDOW_X-4, "YOUR SCORE: %d", my_score), OK, "Error output text!");
		wrefresh(gamewin);
		wgetch(gamewin);
	}
	check_ok(endwin(), OK, "Error closing ncurses!");
	fprintf(log_file, "Exiting from program!");
	fclose(log_file);
	check_wrong(execl(MAIN_MENU_PROGRAM_NAME, MAIN_MENU_PROGRAM_NAME, NULL), -1, "Error changing program execution!");
}
