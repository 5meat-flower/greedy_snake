/*================================================================
*   Copyright (C) 2020 Techyauld LTD. All rights reserved.
*
*   File Name:greedy_snake.c
*   Author:Yang Jian <jyang@techyauld.com>
*   Create Date:2020-08-04
*   Description:
*
================================================================*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define HEAD 'O'
#define ROW 20
#define COL 80
#define VERSION "Alpha"

static volatile bool flag = 1;
static volatile int move = 0;
static volatile int s_up = 0;
static volatile int stop_flag = 0;


static int kbhit(void)
{
        struct termios  tm0, tm1;
        int fd = STDIN_FILENO;
        int c  = -1;

        if(tcgetattr(fd, &tm0) < 0){
                printf("Failed to get terminal attribute\n");
                return -1;
        }

        tm1 = tm0;
        tm1.c_lflag &= ~(ECHO | ECHONL | ECHOE | ECHOK);
        tm1.c_lflag &= ~ICANON;
        tm1.c_cc[VMIN]=1;
        if(tcsetattr(fd, TCSANOW, &tm1) < 0){
                printf("Failed to set terminal attribute\n");
                return -1;
        }

        c = fgetc(stdin);

        if(tcsetattr(fd, TCSANOW, &tm0) < 0){
                printf("Failed to set terminal attribute\n");
                return -1;
	}

	return c;
}

static void get_kb_val(void)
{
	int retval;

	while(flag){
		retval = kbhit();
		switch(retval){
		case 'q':
			flag = 0;
			stop_flag = 0;
			move = 1;
			break;
		case 'w':
		case 'a':
		case 's':
		case 'd':
			move = retval;
			break;
		case ' ':
			s_up =~s_up & 1;
			break;
		case 'p':
			stop_flag = ~stop_flag & 1;
			break;
		default:
			break;
		}
	};
	pthread_exit((void *)1);
}

static void dump_info(void)
{
	printf("\033[0;31;44m Author: YangJian     Version: %s    \033[0m\n", VERSION);
	printf("\033[0;32;44m w    s    a    d     \033[33mspace    \033[31mq    p   \033[0m\n");
	printf("\033[0;32;44m up   down left right \033[33mspeed up \033[31mquit stop\033[0m\n");
}

static int init_map(char **line, int col, int row)
{
	int i;

	for(i = 0; i < ROW; i++){
		*(line+i) = (char *)malloc(COL + 1);
		if(!*(line+i))
			return 1;
		if(!i || i == ROW - 1){
			for(int j = 0; j < COL; j++){
				line[i][j] = '#';
			}
		}else{
			for(int j = 0; j < COL; j++){
				if(!j || j == COL -1)
					line[i][j] = '#';
				else
					line[i][j] = ' ';
			}
		}
	}
	line[row][col] = HEAD;

	return 0;
}

static void set_mark(char **line)
{
	int mark_row, mark_col;

	mark_row = rand() % ROW;
	mark_col = rand() % COL;
	mark_col = (mark_col % 2 ? mark_col-1 : mark_col);
	if(line[mark_row][mark_col] != ' '){
		set_mark(line);
	}else{
		line[mark_row][mark_col] = '$';
	}

}

static int nagate(int move)
{
	switch(move){
	case 'w':
		return 's';
		break;
	case 's':
		return 'w';
		break;
	case 'a':
		return 'd';
		break;
	case 'd':
		return 'a';
		break;
	default:
		return 0;
		break;
	}
}

int main(void)
{
	char **line, **direc;
	pthread_t thread1;
	int col = ((COL/2)%2 ? (COL/2)-1 : (COL/2));
	int col_tail = col;
	int row = ROW/2;
	int row_tail = ROW/2;
	int ret, i, des_col, des_row;
	int mark = 0;
	int length = 1;
	void *retval;

	dump_info();

	ret = pthread_create(&thread1, NULL, (void *)&get_kb_val, NULL);
	if(ret){
		printf("pthread_create failed\n");
		exit(0);
	}

	line = (char **)malloc(sizeof(char *)*ROW);
	direc = (char **)malloc(sizeof(char *)*ROW);
	for(i = 0; i < ROW; i++){
		*(direc+i) = (char *)malloc(COL);
		memset(*(direc+i), 0, COL);
	}
	ret = init_map(line, col, row);
	if(ret){
		printf("init map failed\n");
		exit(0);
	}

	srand((unsigned int)time(NULL));
	set_mark(line);
	for(i = 0; i <= ROW; i++){
		printf("\n");
	}

	while(flag){
		printf("\033[%dA", ROW + 1);
		for(i = 0; i < ROW; i++){
			printf("\033[0;42m%s\033[0m\n", line[i]);
		}
		while(stop_flag){
			usleep(100000);
		}
		if(s_up)
			usleep((400000-300000*length/(ROW*COL))/2);
		else
			usleep(400000-300000*length/(ROW*COL));
		move = (direc[row][col] == nagate(move) ? nagate(move) : move);
		switch(move){
		case 'w':
			des_row = row - 1;
			des_col = col;
			break;
		case 's':
			des_row = row + 1;
			des_col = col;
			break;
		case 'a':
			des_row = row;
			des_col = col - 2;
			break;
		case 'd':
			des_row = row;
			des_col = col + 2;
			break;
		default:
			des_row = row;
			des_col = col;
			break;
		}
		direc[row][col] = move;
		if((line[des_row][des_col] == '#') || (line[des_row][des_col] == '*')){
			break;
		}else if(line[des_row][des_col] == ' '){
			line[des_row][des_col] = line[row][col];
			line[row][col] = line[row_tail][col_tail];
			line[row_tail][col_tail] = ' ';
			switch(direc[row_tail][col_tail]){
			case 'w':
				row_tail--;
				break;
			case 's':
				row_tail++;
				break;
			case 'a':
				col_tail -= 2;
				break;
			case 'd':
				col_tail += 2;
				break;
			default:
				break;
			}
			row = des_row;
			col = des_col;
			direc[row][col] = move;
		}else if(line[des_row][des_col] == '$'){
			line[des_row][des_col] = line[row][col];
			line[row][col] = '*';
			row = des_row;
			col = des_col;
			direc[row][col] = move;
			length++;
			mark += length;
			set_mark(line);
		}
		printf("\033[0;43mScore: \033[5m%d   \033[0;43mLength: \033[5m%d\n", mark, length);
	}
	printf("\033[5;43mGame Over!Final score:%d\033[0m\n",mark);
	printf("Please parse q to quit!\n");
	pthread_join(thread1,(void **)&retval);
	for(int i = 0; i < ROW; i++){
		free(*(line+i));
		free(*(direc+i));
	}
	free(line);
	free(direc);
	return 0;
}
/*========================end of this file======================*/
