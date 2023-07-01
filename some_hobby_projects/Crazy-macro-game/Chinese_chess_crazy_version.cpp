/*
	Author: Ge YangTian 2022-08-14
	Chinese chess. compile this, just like:
	gcc cchess.c
	
	This is a Chinese chess game with AI, just a easy program to examine my thoughts about C.
	I have tortured between C++ and C for almost 1 year, just to understand their's differences,
	and now, I still can't figure out what is right, but I know that, no language is perfect.
	
	Finally, special thanks to my colleague for their useful advices.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <windows.h>

/*  
	windows chcp.
	first, this file is encoded in utf-8, and this file has some Chinese.
	on linux system, we could just compile it and we will get right,
	but, on windows system, you must change cmd's chcp from 936(gbk) into 65001(utf-8), 
	otherwise you will get mojibake.
*/
#define windows_console_chcp(codePageID)\
do{                                     \
	SetConsoleOutputCP(codePageID);     \
	SetConsoleCP(codePageID);           \
}while(0);

/* 
	windows console color.
	using macro below you could easily change the console color on windows system.
	for example:
	
	WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_BLACK, WINDOWS_CONSOLE_COLOR_BRIGHTWHITE);
	
	this will change your font's color into black, and background color is bright white.
*/
#define WINDOWS_CONSOLE_COLOR_BLACK           0
#define WINDOWS_CONSOLE_COLOR_BLUE            1
#define WINDOWS_CONSOLE_COLOR_GREEN           2
#define WINDOWS_CONSOLE_COLOR_CYAN            3
#define WINDOWS_CONSOLE_COLOR_RED             4
#define WINDOWS_CONSOLE_COLOR_MAGENTA         5
#define WINDOWS_CONSOLE_COLOR_BROWN           6
#define WINDOWS_CONSOLE_COLOR_WHITE           7
#define WINDOWS_CONSOLE_COLOR_GRAY            8
#define WINDOWS_CONSOLE_COLOR_BRIGHTCYAN      9
#define WINDOWS_CONSOLE_COLOR_BRIGHTGREEN     10
#define WINDOWS_CONSOLE_COLOR_BRIGHTBLUE      11
#define WINDOWS_CONSOLE_COLOR_BRIGHTRED       12
#define WINDOWS_CONSOLE_COLOR_BRIGHTMAGENTA   13
#define WINDOWS_CONSOLE_COLOR_YELLOW          14
#define WINDOWS_CONSOLE_COLOR_BRIGHTWHITE     15

#define WINDOWS_SET_CONSOLE_COLOR(foreground_color, background_color)                                   \
do{                                                                                                     \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 16 * background_color + foreground_color); \
}while(0);

/* 
	bool.
	ANSI C doesn't have bool type, so I create a one.
*/
#ifndef bool
#define bool    size_t
#define FALSE   0
#define TRUE    1
#endif

/* 
	piece.
	a chess piece has 2 attributes: its side and its type.
	so I think it's a good idea to use bitset to represent these concepts.
*/
#define cchess_piece_side   size_t
#define cchess_piece_type   size_t
#define cchess_piece        size_t

#define cchess_piece_make(side, type)   ((side) | (type))
#define cchess_piece_get_side(p)        ((p) & (size_t)0xf0)
#define cchess_piece_get_type(p)        ((p) & (size_t)0x0f)

#define CCHESS_PIECE_SIDE_BLACK   ((size_t)0x10)
#define CCHESS_PIECE_SIDE_RED     ((size_t)0x20)
#define CCHESS_PIECE_SIDE_EXTRA   ((size_t)0x30)

#define CCHESS_PIECE_TYPE_PAWN      ((size_t)0x01)
#define CCHESS_PIECE_TYPE_CANNON    ((size_t)0x02)
#define CCHESS_PIECE_TYPE_ROOK      ((size_t)0x03)
#define CCHESS_PIECE_TYPE_KNIGHT    ((size_t)0x04)
#define CCHESS_PIECE_TYPE_BISHOP    ((size_t)0x05)
#define CCHESS_PIECE_TYPE_ADVISOR   ((size_t)0x06)
#define CCHESS_PIECE_TYPE_GENERAL   ((size_t)0x07)
#define CCHESS_PIECE_TYPE_EMPTY     ((size_t)0x08)

#define CCHESS_PIECE_BP   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_PAWN)
#define CCHESS_PIECE_BC   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_CANNON)
#define CCHESS_PIECE_BR   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_ROOK)
#define CCHESS_PIECE_BN   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_KNIGHT)
#define CCHESS_PIECE_BB   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_BISHOP)
#define CCHESS_PIECE_BA   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_ADVISOR)
#define CCHESS_PIECE_BG   cchess_piece_make(CCHESS_PIECE_SIDE_BLACK, CCHESS_PIECE_TYPE_GENERAL)
#define CCHESS_PIECE_RP   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_PAWN)
#define CCHESS_PIECE_RC   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_CANNON)
#define CCHESS_PIECE_RR   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_ROOK)
#define CCHESS_PIECE_RN   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_KNIGHT)
#define CCHESS_PIECE_RB   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_BISHOP)
#define CCHESS_PIECE_RA   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_ADVISOR)
#define CCHESS_PIECE_RG   cchess_piece_make(CCHESS_PIECE_SIDE_RED,   CCHESS_PIECE_TYPE_GENERAL)
#define CCHESS_PIECE_EE   cchess_piece_make(CCHESS_PIECE_SIDE_EXTRA, CCHESS_PIECE_TYPE_EMPTY)

/* 
	chess board.
	Chinese chess board is a 9x10 square.
	and below I defined some digits(all begins from 0), some macros to operate the chess board.
	specially, red side is always on the top.
*/
#define CCHESS_BOARD_LENGTH   9
#define CCHESS_BOARD_WIDTH    10
#define CCHESS_BOARD_SQUARE   (CCHESS_BOARD_LENGTH * CCHESS_BOARD_WIDTH)

#define CCHESS_BOARD_BEGIN_ROW      0
#define CCHESS_BOARD_END_ROW        9
#define CCHESS_BOARD_BEGIN_COLUMN   0
#define CCHESS_BOARD_END_COLUMN     8

#define CCHESS_BOARD_9_RED_BEGIN_ROW      0
#define CCHESS_BOARD_9_RED_END_ROW        2

#define CCHESS_BOARD_9_BLACK_BEGIN_ROW      7
#define CCHESS_BOARD_9_BLACK_END_ROW        9

#define CCHESS_BOARD_9_BEGIN_COLUMN   3
#define CCHESS_BOARD_9_END_COLUMN     5

#define CCHESS_BOARD_RED_BORDER_ROW     4
#define CCHESS_BOARD_BLACK_BORDER_ROW   5

typedef struct {
	cchess_piece cp[CCHESS_BOARD_SQUARE];
} cchess_board;

#define cchess_rc_to_pos(r, c)             ((r) * CCHESS_BOARD_LENGTH + (c))
#define cchess_board_get(board, i)         ((board)->cp[(i)])
#define cchess_board_get_rc(board, r, c)   ((board)->cp[((r) * CCHESS_BOARD_LENGTH + (c))])
#define cchess_pos_to_r(i)                 ((i) / CCHESS_BOARD_LENGTH)
#define cchess_pos_to_c(i)                 ((i) % CCHESS_BOARD_LENGTH)

#define cchess_in_red_map(r)      ((r) <= CCHESS_BOARD_RED_BORDER_ROW)
#define cchess_in_black_map(r)    ((r) >= CCHESS_BOARD_BLACK_BORDER_ROW)

#define cchess_in_9(side, r, c)                     \
	((r) >= CCHESS_BOARD_9_##side##_BEGIN_ROW &&    \
	 (r) <= CCHESS_BOARD_9_##side##_END_ROW &&      \
	 (c) >= CCHESS_BOARD_9_BEGIN_COLUMN &&          \
	 (c) <= CCHESS_BOARD_9_END_COLUMN)
	 
#define cchess_board_move(board, f, t)                              \
do{                                                                 \
	cchess_board_get((board), (t)) = cchess_board_get((board), (f));\
	cchess_board_get((board), (f)) = CCHESS_PIECE_EE;               \
}while(0);

#define cchess_board_unmove(board, f, pf, t, pt)\
do{                                             \
	cchess_board_get((board), (t)) = (pt);      \
	cchess_board_get((board), (f)) = (pf);      \
}while(0);

/* 
	chess I/O. 
	here are macros and functions associated with input and output.
*/
#define cchess_is_input_a_move(input)           \
	((input)[0] >= 'a' && (input)[0] <= 'i' &&  \
	 (input)[1] >= '0' && (input)[1] <= '9' &&  \
	 (input)[2] >= 'a' && (input)[0] <= 'i' &&  \
	 (input)[3] >= '0' && (input)[1] <= '9')

#define cchess_is_input_a_cmd(input, cmd) (strcmp((input), (cmd)) == 0)

void  cchess_board_print(cchess_board* bd);
char* cchess_get_user_input();
int   cchess_check_winner(cchess_board* bd);

/* 
	generic dynamic array. 
	ANSI C doesn't support generic programming like C++, 
	but as Bjarne Stroustrup said, C's macro could simulate this, though not perfect.
*/
#define array_generic(T)  \
	typedef struct {      \
		T* buf;           \
		size_t capacity;  \
		size_t length;    \
	} array_##T;  

#define arr_init(arr, T, success)                         \
do{                                                       \
	(arr)->capacity = 5;                                  \
	(arr)->length = 0;                                    \
	(arr)->buf = (T*)malloc(sizeof(T) * (arr)->capacity); \
	(success) = (arr)->buf == NULL ? FALSE : TRUE;        \
} while(0);

#define arr_free(arr)     \
do{                       \
	free((arr)->buf);     \
	(arr)->capacity = 0;  \
	(arr)->length = 0;    \
} while(0);

#define arr_clear(arr)  \
do{                     \
	(arr)->length = 0;  \
}while(0);

#define arr_expand(arr, T, new_size, success)                   \
do{                                                             \
	if ((new_size) <= (arr)->capacity){                         \
		success = FALSE;                                        \
		break;                                                  \
	}                                                           \
															    \
	T* temp = (T*)realloc((arr)->buf, sizeof(T) * (new_size));  \
	if (temp == NULL){                                          \
		success = FALSE;                                        \
		break;                                                  \
	}                                                           \
															    \
	(arr)->buf = temp;                                          \
	(arr)->capacity = (new_size);                               \
	success = TRUE;                                             \
} while(0);

#define arr_add_back(arr, T, elem, success)                 \
do{                                                         \
	if ((arr)->capacity == (arr)->length){                  \
		arr_expand(arr, T, 2*((arr)->capacity), success);   \
		if (success == FALSE){                              \
			break;                                          \
		}                                                   \
	}                                                       \
															\
	memcpy((arr)->buf + (arr)->length, elem, sizeof(T));    \
	++((arr)->length);                                      \
	success = TRUE;                                         \
} while(0);

#define arr_pop_back(arr, T, elem, success)                     \
do{                                                             \
	if ((arr)->length == 0){                                    \
		success = FALSE;                                        \
		break;                                                  \
	}                                                           \
																\
	memcpy(elem, (arr)->buf + (arr)->length - 1, sizeof(T));    \
	--((arr)->length);                                          \
	success = TRUE;                                             \
} while(0);

#define arr_get(arr, i) ((arr)->buf+(i))

/* 
	chess rules and moves generate.
	rules will check if 2 generals are meeting,
	moves generate will store the results into array_cchess_pos_pair, 
	this used the generic dynamic array we have defined before.
*/
typedef struct {
	int from, to;
} cchess_pos_pair;

array_generic(cchess_pos_pair);

bool cchess_rule(cchess_board* bd, int f, int t);
bool cchess_rule_P(cchess_board* bd, int f, int t);
bool cchess_rule_C(cchess_board* bd, int f, int t);
bool cchess_rule_R(cchess_board* bd, int f, int t);
bool cchess_rule_N(cchess_board* bd, int f, int t);
bool cchess_rule_B(cchess_board* bd, int f, int t);
bool cchess_rule_A(cchess_board* bd, int f, int t);
bool cchess_rule_G(cchess_board* bd, int f, int t);

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

bool cchess_are_generals_meeting(cchess_board* bd);
bool cchess_try_move_check_meeting(cchess_board* bd, int f, int t);
bool cchess_are_pos_in_line(cchess_board* bd, int f, int t);
int  cchess_stones_num_in_line(cchess_board* bd, int f, int t);

void cchess_gen_moves(cchess_board* bd, cchess_piece_side s, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_P(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_C(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_R(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_N(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_B(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_A(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);
void cchess_gen_moves_G(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success);

/* main. */
int main(){
	windows_console_chcp(65001);
	
	cchess_board bd = {{
		CCHESS_PIECE_RR, CCHESS_PIECE_RN, CCHESS_PIECE_RB, CCHESS_PIECE_RA, CCHESS_PIECE_RG, CCHESS_PIECE_RA, CCHESS_PIECE_RB, CCHESS_PIECE_RN, CCHESS_PIECE_RR,
		CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, 
		CCHESS_PIECE_EE, CCHESS_PIECE_RC, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_RC, CCHESS_PIECE_EE, 
		CCHESS_PIECE_RP, CCHESS_PIECE_EE, CCHESS_PIECE_RP, CCHESS_PIECE_EE, CCHESS_PIECE_RP, CCHESS_PIECE_EE, CCHESS_PIECE_RP, CCHESS_PIECE_EE, CCHESS_PIECE_RP,
		CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE,		
		CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE,
		CCHESS_PIECE_BP, CCHESS_PIECE_EE, CCHESS_PIECE_BP, CCHESS_PIECE_EE, CCHESS_PIECE_BP, CCHESS_PIECE_EE, CCHESS_PIECE_BP, CCHESS_PIECE_EE, CCHESS_PIECE_BP,
		CCHESS_PIECE_EE, CCHESS_PIECE_BC, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_BC, CCHESS_PIECE_EE, 
		CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE, CCHESS_PIECE_EE,
		CCHESS_PIECE_BR, CCHESS_PIECE_BN, CCHESS_PIECE_BB, CCHESS_PIECE_BA, CCHESS_PIECE_BG, CCHESS_PIECE_BA, CCHESS_PIECE_BB, CCHESS_PIECE_BN, CCHESS_PIECE_BR
	}};
	
	char* input;
	int from, to;
	bool success;
	cchess_pos_pair* pos;
	int fr, fc, tr, tc;
	int winner;
	
	srand((unsigned int)time(NULL));
	array_cchess_pos_pair arr;
	arr_init(&arr, cchess_pos_pair, success);
	
	for (;;){
		arr_clear(&arr);
		cchess_board_print(&bd);
		
		printf("请输入你的走法, 例如a3a4:\n");
		input = cchess_get_user_input();
		
		if (cchess_is_input_a_move(input)){
			from = cchess_rc_to_pos(9-(input[1]-'0'), input[0]-'a');
			to   = cchess_rc_to_pos(9-(input[3]-'0'), input[2]-'a');
			
			if (!cchess_rule(&bd, from, to)){
				printf("%s 这个走法不合规则, 请尝试其它走法.\n", input);
				continue;
			}
			
			cchess_board_move(&bd, from, to);
			cchess_gen_moves(&bd, CCHESS_PIECE_SIDE_RED, &arr, success);
			
			int num = rand() % arr.length;
			pos = arr_get(&arr, num);
			cchess_board_move(&bd, pos->from, pos->to);
			
			fr = cchess_pos_to_r(pos->from);
			fc = cchess_pos_to_c(pos->from);
			tr = cchess_pos_to_r(pos->to);
			tc = cchess_pos_to_c(pos->to);
			printf("AI走步为%c%c%c%c\n", fc+'a', 9-fr+'0', tc+'a', 9-tr+'0');
			
			winner = cchess_check_winner(&bd);
			if (winner == 0){
				continue;
			}
			else if (winner == -1){
				printf("你输了\n");
				break;
			}
			else{
				printf("你赢了\n");
				break;
			}
		}
		else if (cchess_is_input_a_cmd(input, "quit")){
			break;
		}
		else if (cchess_is_input_a_cmd(input, "help")){
			printf("\n 帮助手册: 输入以下单词来进行游玩:\n");
			printf(" help -- 打开帮助手册\n");
			printf(" quit -- 退出游戏\n");
			printf(" a3a4 -- 类似这种输入会被视作走法, 系统会判断该走法是否合法, 例如将军是否会照面\n");
		}
		else{
			printf("抱歉, 没有这个命令, 请尝试给出一个走法，或者quit, help等命令\n");
		}
	}
	
	arr_free(&arr);
	windows_console_chcp(936);
	getchar();
	return 0;
}

/* functions definitions. */
void cchess_board_print(cchess_board* bd){
	#define cchess_inner_piece_to_str(p, str)             \
	do{                                                   \
		if      ((p) == CCHESS_PIECE_BP) { (str) = "卒"; }\
		else if ((p) == CCHESS_PIECE_BC) { (str) = "軳"; }\
		else if ((p) == CCHESS_PIECE_BR) { (str) = "車"; }\
		else if ((p) == CCHESS_PIECE_BN) { (str) = "馬"; }\
		else if ((p) == CCHESS_PIECE_BB) { (str) = "象"; }\
		else if ((p) == CCHESS_PIECE_BA) { (str) = "仕"; }\
		else if ((p) == CCHESS_PIECE_BG) { (str) = "將"; }\
		else if ((p) == CCHESS_PIECE_RP) { (str) = "兵"; }\
		else if ((p) == CCHESS_PIECE_RC) { (str) = "炮"; }\
		else if ((p) == CCHESS_PIECE_RR) { (str) = "车"; }\
		else if ((p) == CCHESS_PIECE_RN) { (str) = "马"; }\
		else if ((p) == CCHESS_PIECE_RB) { (str) = "相"; }\
		else if ((p) == CCHESS_PIECE_RA) { (str) = "士"; }\
		else if ((p) == CCHESS_PIECE_RG) { (str) = "帅"; }\
		else                             { (str) = "<>"; }\
	}while(0);
	
	int i;
	const char* str = NULL;
	cchess_piece p;
	for (i = 0;i < 90;++i){
		if (i == 45){
			printf("\n       楚   河      汉   界");
		}
		
		if (i % 9 == 0){
			WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_BRIGHTGREEN, WINDOWS_CONSOLE_COLOR_BLACK);
			printf("\n %d  ", 9-i/9);
			WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_YELLOW, WINDOWS_CONSOLE_COLOR_BLACK);
		}
		
		p = cchess_board_get(bd, i);
		cchess_inner_piece_to_str(p, str);
		
		if (cchess_piece_get_side(p) == CCHESS_PIECE_SIDE_BLACK){
			WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_BRIGHTBLUE, WINDOWS_CONSOLE_COLOR_BLACK);
		}
		else if (cchess_piece_get_side(p) == CCHESS_PIECE_SIDE_RED){
			WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_BRIGHTRED, WINDOWS_CONSOLE_COLOR_BLACK);
		}
		
		printf("%s", str);
		WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_YELLOW, WINDOWS_CONSOLE_COLOR_BLACK);
		
		if ((i + 1) % 9 != 0){
			printf("-");
		}
	}
	
	WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_BRIGHTGREEN, WINDOWS_CONSOLE_COLOR_BLACK);
	printf("\n\n     a  b  c  d  e  f  g  h  i\n\n");
	WINDOWS_SET_CONSOLE_COLOR(WINDOWS_CONSOLE_COLOR_YELLOW, WINDOWS_CONSOLE_COLOR_BLACK);
	#undef cchess_inner_piece_to_str
}

char* cchess_get_user_input(){
	#define CCHESS_BUF_SIZE   10
	
	static char buf[CCHESS_BUF_SIZE];
	memset(buf, '\0', CCHESS_BUF_SIZE);
	
	if (fgets(buf, CCHESS_BUF_SIZE, stdin) != NULL){
		int len = strlen(buf);
		
		if (len == CCHESS_BUF_SIZE-1 && buf[CCHESS_BUF_SIZE-2] != '\n'){
			char c;
			while ((c = getchar()) != '\n' && c != EOF);
		}
		else{
			buf[len-1] = '\0';
		}
	}
	
	return buf;
	#undef CCHESS_USER_INPUT_BUFFER_SIZE
}

int cchess_check_winner(cchess_board* bd){
	int red_general_pos = -1;
	int black_general_pos = -1;
	
	int i;
	for (i = 0;i < CCHESS_BOARD_SQUARE;++i){
		if (cchess_board_get(bd, i) == CCHESS_PIECE_RG){
			red_general_pos = i;
		}
		
		if (cchess_board_get(bd, i) == CCHESS_PIECE_BG){
			black_general_pos = i;
		}
	}
	
	if (red_general_pos != -1 && black_general_pos != -1){
		return 0;
	}
	
	if (red_general_pos != -1){
		return -1;
	}
	else{
		return 1;
	}
}

bool cchess_rule(cchess_board* bd, int f, int t){
	if (f == t){
		return FALSE;
	}
	
	cchess_piece pf = cchess_board_get(bd, f);
	cchess_piece pt = cchess_board_get(bd, t);
	
	if (cchess_piece_get_side(pf) == cchess_piece_get_side(pt)){
		return FALSE;
	}
	
	bool result;
	switch(cchess_piece_get_type(pf)){
		case CCHESS_PIECE_TYPE_PAWN:    result = cchess_rule_P(bd, f, t); break;
		case CCHESS_PIECE_TYPE_CANNON:  result = cchess_rule_C(bd, f, t); break;
		case CCHESS_PIECE_TYPE_ROOK:    result = cchess_rule_R(bd, f, t); break;
		case CCHESS_PIECE_TYPE_KNIGHT:  result = cchess_rule_N(bd, f, t); break;
		case CCHESS_PIECE_TYPE_BISHOP:  result = cchess_rule_B(bd, f, t); break;
		case CCHESS_PIECE_TYPE_ADVISOR: result = cchess_rule_A(bd, f, t); break;
		case CCHESS_PIECE_TYPE_GENERAL: result = cchess_rule_G(bd, f, t); break;
		default:                        result = FALSE; break;
	}
	
	return result && !cchess_try_move_check_meeting(bd, f, t);
}

bool cchess_rule_P(cchess_board* bd, int f, int t){
	cchess_piece p = cchess_board_get(bd, f);
	
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	int sum = 10*abs(fr-tr) + abs(fc-tc);
	
	if (cchess_piece_get_side(p) == CCHESS_PIECE_SIDE_RED){
		if (fc > tc){
			return FALSE;
		}
		
		if (cchess_in_red_map(fr)){
			return sum == 10;
		}
		else{
			return sum == 10 || sum == 1;
		}
	}
	else{
		if (fc < tc){
			return FALSE;
		}
		
		if (cchess_in_black_map(fr)){
			return sum == 10;
		}
		else{
			return sum == 10 || sum == 1;
		}
	}
}

bool cchess_rule_C(cchess_board* bd, int f, int t){
	if (!cchess_are_pos_in_line(bd, f, t)){
		return FALSE;
	}
	
	int num = cchess_stones_num_in_line(bd, f, t);
	return num == 0 || (num == 1 && cchess_board_get(bd, t) != CCHESS_PIECE_EE);
}

bool cchess_rule_R(cchess_board* bd, int f, int t){
	if (!cchess_are_pos_in_line(bd, f, t)){
		return FALSE;
	}
	
	int num = cchess_stones_num_in_line(bd, f, t);
	return num == 0;
}

bool cchess_rule_N(cchess_board* bd, int f, int t){
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	int sum = 10*abs(fr-tr) + abs(fc-tc);
	
	if (sum == 12){
		return cchess_board_get_rc(bd, fr, (fc+tc)/2) == CCHESS_PIECE_EE;
	}
	else if (sum == 21){
		return cchess_board_get_rc(bd, (fr+tr)/2, fc) == CCHESS_PIECE_EE;
	}
	else{
		return FALSE;
	}
}

bool cchess_rule_B(cchess_board* bd, int f, int t){
	cchess_piece p = cchess_board_get(bd, f);
	
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	
	if (cchess_piece_get_side(p) == CCHESS_PIECE_SIDE_RED){
		if (!cchess_in_red_map(tr)){
			return FALSE;
		}
	}
	else{
		if (!cchess_in_black_map(tr)){
			return FALSE;
		}
	}
	
	if (cchess_board_get_rc(bd, (fr+tr)/2, (fc+tc)/2) != CCHESS_PIECE_EE){
		return FALSE;
	}
	
	int sum = 10*abs(fr-tr) + abs(fc-tc);
	return sum == 22;
}

bool cchess_rule_A(cchess_board* bd, int f, int t){
	cchess_piece p = cchess_board_get(bd, f);
	
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	
	if (cchess_piece_get_side(p) == CCHESS_PIECE_SIDE_RED){
		if (!cchess_in_9(RED, tr, tc)){
			return FALSE;
		}
	}
	else{
		if (!cchess_in_9(BLACK, tr, tc)){
			return FALSE;
		}
	}
	
	int sum = 10*abs(fr-tr) + abs(fc-tc);
	return sum == 11;
}

bool cchess_rule_G(cchess_board* bd, int f, int t){
	cchess_piece p = cchess_board_get(bd, f);
	
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	
	if (cchess_piece_get_side(p) == CCHESS_PIECE_SIDE_RED){
		if (!cchess_in_9(RED, tr, tc)){
			return FALSE;
		}
	}
	else{
		if (!cchess_in_9(BLACK, tr, tc)){
			return FALSE;
		}
	}
	
	int sum = 10*abs(fr-tr) + abs(fc-tc);
	return sum == 1 || sum == 10;
}

bool cchess_are_generals_meeting(cchess_board* bd){
	cchess_piece p;
	int r, c;
	for (r = CCHESS_BOARD_9_RED_BEGIN_ROW;r <= CCHESS_BOARD_9_RED_END_ROW;++r){
		for (c = CCHESS_BOARD_9_BEGIN_COLUMN;c <= CCHESS_BOARD_9_END_COLUMN;++c){
			p = cchess_board_get_rc(bd, r, c);
			if (cchess_piece_get_type(p) == CCHESS_PIECE_TYPE_GENERAL){
				goto out;
			}
		}
	}
	
	out:
	for (r = r+1;r <= CCHESS_BOARD_END_ROW;++r){
		p = cchess_board_get_rc(bd, r, c);
		if (p != CCHESS_PIECE_EE){
			break;
		}
	}
	
	return (cchess_piece_get_type(p) == CCHESS_PIECE_TYPE_GENERAL);
}

bool cchess_try_move_check_meeting(cchess_board* bd, int f, int t){
	cchess_piece pf = cchess_board_get(bd, f);
	cchess_piece pt = cchess_board_get(bd, t);
	
	cchess_board_move(bd, f, t);
	bool result = cchess_are_generals_meeting(bd);
	cchess_board_unmove(bd, f, pf, t, pt);
	return result;
}

bool cchess_are_pos_in_line(cchess_board* bd, int f, int t){
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	
	return (fr == tr || fc == tc);
}

int cchess_stones_num_in_line(cchess_board* bd, int f, int t){
	int fr = cchess_pos_to_r(f);
	int fc = cchess_pos_to_c(f);
	int tr = cchess_pos_to_r(t);
	int tc = cchess_pos_to_c(t);
	
	int begin, end;
	int sum = 0;
	
	if (fr == tr){
		begin = min(fc, tc);
		end   = max(fc, tc);
		
		for (begin = begin+1;begin < end;++begin){
			if (cchess_board_get_rc(bd, fr, begin) != CCHESS_PIECE_EE){
				++sum;
			}
		}
		
		return sum;
	}
	else{
		begin = min(fr, tr);
		end   = max(fr, tr);
		
		for (begin = begin+1;begin < end;++begin){
			if (cchess_board_get_rc(bd, begin, fc) != CCHESS_PIECE_EE){
				++sum;
			}
		}
		
		return sum;
	}
}

void cchess_gen_moves(cchess_board* bd, cchess_piece_side s, array_cchess_pos_pair* buf, bool success){
	if (s == CCHESS_PIECE_SIDE_EXTRA){
		return;
	}
	
	cchess_piece p;
	int i;
	
	for (i = 0;i < CCHESS_BOARD_SQUARE;++i){
		p = cchess_board_get(bd, i);
		if (cchess_piece_get_side(p) == s){
			switch(cchess_piece_get_type(p)){
				case CCHESS_PIECE_TYPE_PAWN:    cchess_gen_moves_P(bd, i, buf, success); break;
				case CCHESS_PIECE_TYPE_CANNON:  cchess_gen_moves_C(bd, i, buf, success); break;
				case CCHESS_PIECE_TYPE_ROOK:    cchess_gen_moves_R(bd, i, buf, success); break;
				case CCHESS_PIECE_TYPE_KNIGHT:  cchess_gen_moves_N(bd, i, buf, success); break;
				case CCHESS_PIECE_TYPE_BISHOP:  cchess_gen_moves_B(bd, i, buf, success); break;
				case CCHESS_PIECE_TYPE_ADVISOR: cchess_gen_moves_A(bd, i, buf, success); break;
				case CCHESS_PIECE_TYPE_GENERAL: cchess_gen_moves_G(bd, i, buf, success); break;
				default:                        break;
			}
		}
	}
}

void cchess_gen_moves_P(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){	
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side s = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(rr, cc, side)                                                  \
	do{                                                                                         \
		temp = cchess_board_get_rc(bd, rr, cc);                                                 \
		if (cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){ \
			pair.to = cchess_rc_to_pos(rr, cc);                                                 \
			arr_add_back(buf, cchess_pos_pair, &pair, success);                                 \
		}                                                                                       \
	}while(0);
	
	if (s == CCHESS_PIECE_SIDE_RED){
		if (cchess_in_red_map(r)){
			cchess_inner_try_add(r+1, c, CCHESS_PIECE_SIDE_RED);
		}
		else{
			if (r+1 <= CCHESS_BOARD_END_ROW)    { cchess_inner_try_add(r+1, c, CCHESS_PIECE_SIDE_RED); }
			if (c+1 <= CCHESS_BOARD_END_COLUMN) { cchess_inner_try_add(r, c+1, CCHESS_PIECE_SIDE_RED); }
			if (c-1 >= CCHESS_BOARD_BEGIN_COLUMN)  { cchess_inner_try_add(r, c-1, CCHESS_PIECE_SIDE_RED); }
		}
	}
	else{
		if (cchess_in_black_map(r)){
			cchess_inner_try_add(r-1, c, CCHESS_PIECE_SIDE_BLACK);
		}
		else{
			if (r-1 >= CCHESS_BOARD_BEGIN_ROW)  { cchess_inner_try_add(r-1, c, CCHESS_PIECE_SIDE_BLACK); }
			if (c+1 <= CCHESS_BOARD_END_COLUMN) { cchess_inner_try_add(r, c+1, CCHESS_PIECE_SIDE_BLACK); } 
			if (c-1 >= CCHESS_BOARD_BEGIN_COLUMN)  { cchess_inner_try_add(r, c-1, CCHESS_PIECE_SIDE_BLACK); }
		}
	}
	
	#undef cchess_inner_try_add
}

void cchess_gen_moves_C(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side side = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(op0, op1, op2, rr, cc)                                                                            \
	do{                                                                                                                            \
		op0;                                                                                                                       \
		for (;op1;op2){                                                                                                            \
			temp = cchess_board_get_rc(bd, rr, cc);                                                                                \
			if (temp == CCHESS_PIECE_EE){                                                                                          \
				if (!cchess_try_move_check_meeting(bd, rr, cc)){                                                                   \
					pair.to = cchess_rc_to_pos(rr, cc);                                                                            \
					arr_add_back(buf, cchess_pos_pair, &pair, success);                                                            \
				}                                                                                                                  \
			}                                                                                                                      \
			else{                                                                                                                  \
				break;                                                                                                             \
			}                                                                                                                      \
		}                                                                                                                          \
																		                                                           \
		op2;                                                                                                                       \
		for (;op1;op2){                                                                                                            \
			temp = cchess_board_get_rc(bd, rr, cc);                                                                                \
			if (temp != CCHESS_PIECE_EE && cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){     \
				pair.to = cchess_rc_to_pos(rr, cc);                                                                                \
				arr_add_back(buf, cchess_pos_pair, &pair, success);                                                                \
				break;                                                                                                             \
			}                                                                                                                      \
		}                                                                                                                          \
	}while(0);
	
	cchess_inner_try_add(int rr = r+1, rr <= CCHESS_BOARD_END_ROW,      ++rr, rr, c);
	cchess_inner_try_add(int rr = r-1, rr >= CCHESS_BOARD_BEGIN_ROW,    --rr, rr, c);
	cchess_inner_try_add(int cc = c+1, cc <= CCHESS_BOARD_END_COLUMN,   ++cc,  r, cc);
	cchess_inner_try_add(int cc = c-1, cc >= CCHESS_BOARD_BEGIN_COLUMN, --cc,  r, cc);

	#undef cchess_inner_try_add
}

void cchess_gen_moves_R(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side side = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(op0, op1, op2, rr, cc)                                                 \
	do{                                                                                                 \
		op0;                                                                                            \
		for (;op1;op2){                                                                                 \
			temp = cchess_board_get_rc(bd, rr, cc);                                                     \
			if (temp == CCHESS_PIECE_EE && !cchess_try_move_check_meeting(bd, rr, cc)){                 \
				pair.to = cchess_rc_to_pos(rr, cc);                                                     \
				arr_add_back(buf, cchess_pos_pair, &pair, success);                                     \
			}                                                                                           \
			else{                                                                                       \
				if (cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){ \
					pair.to = cchess_rc_to_pos(rr, cc);                                                 \
					arr_add_back(buf, cchess_pos_pair, &pair, success);                                 \
				}                                                                                       \
																	                                    \
				break;                                                                                  \
			}                                                                                           \
		}                                                                                               \
	}while(0);
	
	cchess_inner_try_add(int rr = r+1, rr <= CCHESS_BOARD_END_ROW, ++rr, rr, c);
	cchess_inner_try_add(int rr = r-1, rr >= CCHESS_BOARD_BEGIN_ROW, --rr, rr, c);
	cchess_inner_try_add(int cc = c+1, cc <= CCHESS_BOARD_END_COLUMN, ++cc, r, cc);
	cchess_inner_try_add(int cc = c-1, cc >= CCHESS_BOARD_BEGIN_COLUMN, --cc, r, cc);

	#undef cchess_inner_try_add
}

void cchess_gen_moves_N(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side side = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(rr, cc)                                                        \
	do{                                                                                         \
		temp = cchess_board_get_rc(bd, rr, cc);                                                 \
		if (cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){ \
			pair.to = cchess_rc_to_pos(rr, cc);                                                 \
			arr_add_back(buf, cchess_pos_pair, &pair, success);                                 \
		}                                                                                       \
	}while(0);
	
	if (r+2 <= CCHESS_BOARD_END_ROW   && c-1 >= CCHESS_BOARD_BEGIN_COLUMN){ cchess_inner_try_add(r+2, c-1); }
	if (r+2 <= CCHESS_BOARD_END_ROW   && c+1 <= CCHESS_BOARD_END_COLUMN)  { cchess_inner_try_add(r+2, c+1); }
	if (r-2 >= CCHESS_BOARD_BEGIN_ROW && c-1 >= CCHESS_BOARD_BEGIN_COLUMN){ cchess_inner_try_add(r-2, c-1); }
	if (r-2 >= CCHESS_BOARD_BEGIN_ROW && c+1 <= CCHESS_BOARD_END_COLUMN)  { cchess_inner_try_add(r-2, c+1); }
	if (r+1 <= CCHESS_BOARD_END_ROW   && c+2 <= CCHESS_BOARD_END_COLUMN)  { cchess_inner_try_add(r+1, c+2); }
	if (r+1 <= CCHESS_BOARD_END_ROW   && c-2 >= CCHESS_BOARD_BEGIN_COLUMN){ cchess_inner_try_add(r+1, c-2); }
	if (r-1 >= CCHESS_BOARD_BEGIN_ROW && c+2 <= CCHESS_BOARD_END_COLUMN)  { cchess_inner_try_add(r-1, c+2); }
	if (r-1 >= CCHESS_BOARD_BEGIN_ROW && c-2 >= CCHESS_BOARD_BEGIN_COLUMN){ cchess_inner_try_add(r-1, c-2); }
	
	#undef cchess_inner_try_add
}

void cchess_gen_moves_B(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side side = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(rr, cc)                                                        \
	do{                                                                                         \
		temp = cchess_board_get_rc(bd, rr, cc);                                                 \
		if (cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){ \
			pair.to = cchess_rc_to_pos(rr, cc);                                                 \
			arr_add_back(buf, cchess_pos_pair, &pair, success);                                 \
		}                                                                                       \
	}while(0);
	
	if (r+2 <= CCHESS_BOARD_END_ROW   && c+2 <= CCHESS_BOARD_END_COLUMN)  { cchess_inner_try_add(r+2, c+2); }
	if (r+2 <= CCHESS_BOARD_END_ROW   && c-2 >= CCHESS_BOARD_BEGIN_COLUMN){ cchess_inner_try_add(r+2, c-2); }
	if (r-2 >= CCHESS_BOARD_BEGIN_ROW && c+2 <= CCHESS_BOARD_END_COLUMN)  { cchess_inner_try_add(r-2, c+2); }
	if (r-2 >= CCHESS_BOARD_BEGIN_ROW && c-2 >= CCHESS_BOARD_BEGIN_COLUMN){ cchess_inner_try_add(r-2, c-2); }
	
	#undef cchess_inner_try_add
}

void cchess_gen_moves_A(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side s = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(rr, cc, side)                                                  \
	do{                                                                                         \
		temp = cchess_board_get_rc(bd, rr, cc);                                                 \
		if (cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){ \
			pair.to = cchess_rc_to_pos(rr, cc);                                                 \
			arr_add_back(buf, cchess_pos_pair, &pair, success);                                 \
		}                                                                                       \
	}while(0);
	
	if (s == CCHESS_PIECE_SIDE_RED){
		if (r+1 <= CCHESS_BOARD_9_RED_END_ROW   && c+1 <= CCHESS_BOARD_9_END_COLUMN)  { cchess_inner_try_add(r+1, c+1, s); }
		if (r+1 <= CCHESS_BOARD_9_RED_END_ROW   && c-1 >= CCHESS_BOARD_9_BEGIN_COLUMN){ cchess_inner_try_add(r+1, c-1, s); }
		if (r-1 >= CCHESS_BOARD_9_RED_BEGIN_ROW && c+1 <= CCHESS_BOARD_9_END_COLUMN)  { cchess_inner_try_add(r-1, c+1, s); }
		if (r-1 >= CCHESS_BOARD_9_RED_BEGIN_ROW && c-1 >= CCHESS_BOARD_9_BEGIN_COLUMN){ cchess_inner_try_add(r-1, c-1, s); }
	}
	else{
		if (r+1 <= CCHESS_BOARD_9_BLACK_END_ROW   && c+1 <= CCHESS_BOARD_9_END_COLUMN)  { cchess_inner_try_add(r+1, c+1, s); }
		if (r+1 <= CCHESS_BOARD_9_BLACK_END_ROW   && c-1 >= CCHESS_BOARD_9_BEGIN_COLUMN){ cchess_inner_try_add(r+1, c-1, s); }
		if (r-1 >= CCHESS_BOARD_9_BLACK_BEGIN_ROW && c+1 <= CCHESS_BOARD_9_END_COLUMN)  { cchess_inner_try_add(r-1, c+1, s); }
		if (r-1 >= CCHESS_BOARD_9_BLACK_BEGIN_ROW && c-1 >= CCHESS_BOARD_9_BEGIN_COLUMN){ cchess_inner_try_add(r-1, c-1, s); }
	}
	
	#undef cchess_inner_try_add
}

void cchess_gen_moves_G(cchess_board* bd, int i, array_cchess_pos_pair* buf, bool success){	
	int r = cchess_pos_to_r(i);
	int c = cchess_pos_to_c(i);
	cchess_piece_side s = cchess_piece_get_side(cchess_board_get(bd, i));
	
	cchess_piece temp;
	cchess_pos_pair pair = { i, 0 };
	
	#define cchess_inner_try_add(rr, cc, side)                                                  \
	do{                                                                                         \
		temp = cchess_board_get_rc(bd, rr, cc);                                                 \
		if (cchess_piece_get_side(temp) != side && !cchess_try_move_check_meeting(bd, rr, cc)){ \
			pair.to = cchess_rc_to_pos(rr, cc);                                                 \
			arr_add_back(buf, cchess_pos_pair, &pair, success);                                 \
		}                                                                                       \
	}while(0);
	
	if (s == CCHESS_PIECE_SIDE_RED){
		if (r+1 <= CCHESS_BOARD_9_RED_END_ROW)   { cchess_inner_try_add(r+1, c, s); }
		if (c-1 >= CCHESS_BOARD_9_BEGIN_COLUMN)  { cchess_inner_try_add(r, c-1, s); }
		if (r-1 >= CCHESS_BOARD_9_RED_BEGIN_ROW) { cchess_inner_try_add(r-1, c, s); }
		if (c+1 <= CCHESS_BOARD_9_END_COLUMN)    { cchess_inner_try_add(r, c+1, s); }
	}
	else{
		if (r+1 <= CCHESS_BOARD_9_BLACK_END_ROW)   { cchess_inner_try_add(r+1, c, s); }
		if (c-1 >= CCHESS_BOARD_9_BEGIN_COLUMN)    { cchess_inner_try_add(r, c-1, s); }
		if (r-1 >= CCHESS_BOARD_9_BLACK_BEGIN_ROW) { cchess_inner_try_add(r-1, c, s); }
		if (c+1 <= CCHESS_BOARD_9_END_COLUMN)      { cchess_inner_try_add(r, c+1, s); }
	}
	
	#undef cchess_inner_try_add
}
