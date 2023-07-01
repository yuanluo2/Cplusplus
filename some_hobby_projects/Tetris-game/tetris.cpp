#pragma clang diagnostic ignored "-Winvalid-source-encoding"    // if you use clang to compile , open this to hide wrong messages . And more , save this file as ANSI encoding .
#include <iostream>
#include <random>            // for Make_shape
#include <chrono>
#include <bitset>            // std::bitset<> is very useful for this project .
#include <thread>            // for std::this_thread
#include <windows.h>         // for Hide_cursor      
#include <conio.h>           // for _getch()

using namespace std;
using namespace std::chrono;

constexpr int ROW = 29;
constexpr int COL = 16;

/*
*  We need 7 shapes : I T L J Z S O .
*  Each shape can be rotated 90 degrees clockwise or counterclockwise to get 4 states .
*  Thus we get a 7 x 4 array .
*/
const bitset<16> tetris_shapes[7][4] = {   // bitset<16> , 7 , 4 is fixed, do not modify
	{ 0x00F0, 0x2222, 0x00F0, 0x2222 },  // I         =>  for example : 0x0072  =>   0 0 0 0      0x0264   =>   0 0 0 0
	{ 0x0072, 0x0262, 0x0270, 0x0232 },  // T                              (T)       0 0 0 0	     (N)        0 0 1 0
	{ 0x0223, 0x0074, 0x0622, 0x0170 },  // L                                        0 1 1 1                    0 1 1 0
	{ 0x0226, 0x0470, 0x0322, 0x0071 },  // J                                        0 0 1 0                    0 1 0 0
	{ 0x0063, 0x0264, 0x0063, 0x0264 },  // Z                            
	{ 0x006C, 0x0462, 0x006C, 0x0462 },  // S                           split every 16 bits into a 4 x 4 array , and focus on number '1' , what do they like ? 
	{ 0x0660, 0x0660, 0x0660, 0x0660 }   // O
};

bitset<16> tetris_map[ROW] = {
	0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001,              //  you will get a map like :  1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
	0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001,                                    //       1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
	0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001,                                    //       ...
	0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001,                                    //       ...    other 26 lines.
	0xFFFF                                                                                     //       1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
};       // if you modify the ROW , remember to add 0x8001 .

/*
*  Using C++'s random engine to generate a kind of tetris_shape .
*  This class and the Hide_cursor class are designed to used like a function call .
*/
class Make_shape {
	default_random_engine          dre;
	uniform_int_distribution<int>  uid1{ 0,6 };      
	uniform_int_distribution<int>  uid2{ 0,3 };
public:
	Make_shape() {
		auto now = system_clock::now();
		long long seed = duration_cast<milliseconds>(now.time_since_epoch()).count();
		dre = default_random_engine(static_cast<unsigned int>(seed));          // this is a mandotary operation .
	}

	std::pair<int, int> operator()() noexcept {
		return make_pair( uid1(dre), uid2(dre) );
	}
};

/*
* Hide cmd's cursor to get a pretty output .Only works on Windows.
*/
class Hide_cursor {
	HANDLE cmd_output_handle = nullptr;
public:
	Hide_cursor() {
		CONSOLE_CURSOR_INFO cursorInfo{ 1, FALSE };       // Get more details on MSDN .
		cmd_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleCursorInfo(cmd_output_handle, &cursorInfo);
	}

	~Hide_cursor() {
		if (cmd_output_handle != nullptr)
			CloseHandle(cmd_output_handle);
	}

	void operator()() noexcept {
		SetConsoleCursorPosition(cmd_output_handle, COORD{ 0,0 });
	}
};

class Square {
	bool stop;        //  if this square's bottom touch any other square , this will be set to true . Cause this time , the square really stops , so I use the word 'stop'.
public:
	int r, c;         //  r, c are used to determine tetris_shapes . 
	int row = 0;      //  row, col represent the coordinates in tetris_map .
	int col = 6;

	Square(const std::pair<int, int> &choice)
		: r{ choice.first }, c{ choice.second }, stop{ false }
	{}

	bool is_stop() const noexcept {
		return stop;
	}

	bool check_collision() noexcept {       // Note : Any two squares overlapping are considered as collisions
		for (int i = 0; i < 16; ++i)
			if ( (tetris_shapes[r][c])[i] && tetris_map[row + i / 4][col + i % 4] )
				return true;

		return false;
	}

	void clock_wise() noexcept {
		c = (c == 3) ? 0 : ++c;         // 3 means the last column.

		if (check_collision()) 
			c = (c == 0) ? 3 : --c;         // 0 means the first column.
	}

	void counter_clock_wise() noexcept {
		c = (c == 0) ? 3 : --c;

		if (check_collision())
			c = (c == 3) ? 0 : ++c;
	}

	void left() noexcept {
		--col;
		if (check_collision())
			++col;
	}

	void right() noexcept {
		++col;
		if (check_collision())
			--col;
	}

	void down() noexcept {
		++row;
		if (check_collision()) {
			stop = true;
			--row;

			for (int i = 0; i < COL; ++i)
				if ( (tetris_shapes[r][c])[i] )
					tetris_map[row + i / 4][col + i % 4] = 1;
		}
	}
};

class Manager {
	Hide_cursor hide;
	Make_shape  make;
	Square      square;
	bool        dead;

	/*
	*  Garbled only happens here . Remember to save this file as ANSI encoding , or just copy it to Visual studio .
	*/
	void print_tetris_map() noexcept {
		hide();       // Every time we print the tetris_map , we should call this function to reposition the cursor and hide it .
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				if (r >= square.row && r < square.row + 4 && c >= square.col && c < square.col + 4) {
					if ( tetris_map[r][c] || ( tetris_shapes[square.r][square.c] )[4 * (r - square.row) + (c - square.col)] )
						cout << "¡ö";
					else
						cout << "  ";    // 2 spaces .
				}
				else {
					if ( tetris_map[r][c] )
						cout << "¡õ";
					else
						cout << "  ";    // 2 spaces .
				}
			}

			cout << endl;
		}          // Actually , this function has a little logic error , but its output is beautiful , so I kept it .
	}

	void erase_line() noexcept {
		int count = 0;             // Note : start-end is not necessarily equal to count .
		int start = 0;
		int end = 0;

		for (int r = ROW - 2; r > 0; --r) {
			if (tetris_map[r] == 0xFFFF) {     // if a line is 0xFFFF , this line is full , we set it as "start" .
				if (count == 0)
					start = r;

				++count;      // when a line is full , the line on it may be full , too , so we have to record that .
			}
			else if (tetris_map[r] == 0x8001) {      // 0x8001 means this line is empty , there's no need to check it and the lines on it , so we set it as "end".
				end = r;
				break;
			}
		}

		if (count > 0)
			for (int r = start; r > end; --r)
				tetris_map[r] = (r >= count) ? tetris_map[r - count] : 0x8001;
	}

	void key_control(const char &key) noexcept {
		switch (key) {
			case 'a':              // I prefer to control with my left hand .
				square.left();
				break;
			case 'd':
				square.right();
				break;
			case 's':
				square.down();
				square.down();      // Let the square fall faster .
				break;
			case 'w':
				square.counter_clock_wise();
				break;
			case 'e':           // I hardly use this key .
				square.clock_wise();
				break;
		}
	}

public:
	Manager()
		: square{ make() }, dead{ false }
	{}

	void play() noexcept {
		auto now = system_clock::now();

		while (!dead) {
			while (_kbhit())      // check if the user presses the key .
				key_control(_getch());

			if (square.is_stop())
				square = Square{ make() };

			if (square.check_collision())
				dead = true;        // this time , square has touch the map's top .

			auto end = system_clock::now();
			if (duration_cast<milliseconds>(end - now).count() > 450) {
				square.down();
				now = end;
			}

			erase_line();
			print_tetris_map();
			
			/*
			*   Note : Add this to relax CPU for a short break , when you run without this code , CPU may occupies up to 30% . 
			*          In order to make the output smoother , in the function : key_control() , case 's' , I use 2 square.down() .
			*/
			this_thread::sleep_for(25ms);     
		}                                     
	}
};

int main() {
	system("color E");        // I think light yellow is suit for this game . If you want other colors , try "color -h" in your cmd .
	Manager man;
	man.play();
}
