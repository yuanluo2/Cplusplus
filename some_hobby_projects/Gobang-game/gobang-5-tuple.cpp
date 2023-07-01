#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <stack>
#include <algorithm>

using namespace std;

const int SIZE = 15;        // chess board is 15x15.
const char ch_EMPTY = '.';
const char ch_human = 'o';
const char ch_ai = 'x';

const vector<string> chess_board{
	"     a b c d e f g h i j k l m n o",
	"   |-------------------------------|",
	" a | . . . . . . . . . . . . . . . | a",
	" b | . . . . . . . . . . . . . . . | b",
	" c | . . . . . . . . . . . . . . . | c",
	" d | . . . . . . . . . . . . . . . | d",
	" e | . . . . . . . . . . . . . . . | e",
	" f | . . . . . . . . . . . . . . . | f",
	" g | . . . . . . . . . . . . . . . | g",
	" h | . . . . . . . . . . . . . . . | h",
	" i | . . . . . . . . . . . . . . . | i",
	" j | . . . . . . . . . . . . . . . | j",
	" k | . . . . . . . . . . . . . . . | k",
	" l | . . . . . . . . . . . . . . . | l",
	" m | . . . . . . . . . . . . . . . | m",
	" n | . . . . . . . . . . . . . . . | n",
	" o | . . . . . . . . . . . . . . . | o",
	"   |-------------------------------|",
	"     a b c d e f g h i j k l m n o"
};

/*
	This a set of platform specific functions.
	for example, "cls" is used on windows, "clear" is used on linux, which are used to clear screen. They're not part of standard C++.
*/
struct platform_specific{
	inline static void clear_screen(){
		system("cls");       
	}

	inline static void pause(){
		system("pause");
	}
};

/*
	This class is used for processing chess board.
*/
class Chess{
	vector<string> copy_board;
public:
	Chess() : copy_board{ chess_board } {}
	~Chess(){}
	
	inline void print() noexcept {     
		std::for_each(copy_board.cbegin(), copy_board.cend(), [](const string &line){ cout << line << "\n"; });	
	}
	
	inline bool check_subscript(size_t r, size_t c) noexcept {
		return (r < SIZE && c < SIZE);
	}
	
	inline char pos(size_t r, size_t c) noexcept {         // return character at (r, c). This function does't check subscript.
		return copy_board[r + 2][c * 2 + 5];
	}
	
	inline void reset(size_t r, size_t c) noexcept {
		if (!check_subscript(r, c))
			return;
		
		copy_board[r + 2][c * 2 + 5] = ch_EMPTY;
	}
	
	inline bool is_empty(size_t r, size_t c) noexcept {
		if (!check_subscript(r, c))
			return false;
		
		return ( pos(r, c) == ch_EMPTY );
	}
	
	inline bool pos(size_t r, size_t c, char ch) noexcept {         // modify character at (r, c).
		if (!is_empty(r, c))
			return false;
		
		copy_board[r + 2][c * 2 + 5] = ch;
		return true;
	}
	
	inline bool is_equal(size_t r, size_t c, char ch) noexcept {         // compare character at (r, c) with ch.
		if (!check_subscript(r, c))
			return false;
		
		return ( pos(r, c) == ch );
	}
	
	inline bool check_one_direction(size_t r, size_t c, size_t rr, size_t cc, char ch) noexcept {
		int counter = 0;
		
		size_t temp_a = r;
		size_t temp_b = c;
		
		while(is_equal(temp_a, temp_b, ch)){
			++counter;
			
			temp_a += rr;
			temp_b += cc;
		}
		
		size_t temp_c = r - rr;
		size_t temp_d = c - cc;
		
		while(is_equal(temp_c, temp_d, ch)){
			++counter;
			
			temp_c -= rr;
			temp_d -= cc;
		}
		
		if (counter >= 5)
			return true;
		
		return false;
	}
	
	/*
		-1,-1  -1,0  -1,+1
		 0,-1   0,0   0,+1
		+1,-1  +1,0  +1,+1
	*/
	inline bool check_player(size_t r, size_t c) noexcept {         // check 4 directions.
		if (!check_subscript(r, c))
			return false;
		
		char ch = pos(r, c);
		
		if (check_one_direction(r, c, -1, -1, ch))
			return true;
		if (check_one_direction(r, c, -1,  0, ch))
			return true;
		if (check_one_direction(r, c, -1,  1, ch))
			return true;
		if (check_one_direction(r, c,  0, -1, ch))
			return true;
		
		return false;
	}
};

/*
	AI
*/
class AI{
	const int ELEVEN = 11;     
	const int FOUR =  4;
	
	Chess &chess;
	array<array<long, SIZE>, SIZE> scores{};       // 15x15 scores board.
	
	inline long evaluate(int human, int ai) noexcept {     // This function is very special, I have learned this on web.
		if (human > 0 && ai > 0)
			return 0;
		if (human == 0 && ai == 0)
			return 7;
		
		if (human){
			switch (human){
				case 1:
					return 15;
				case 2:
					return 400;
				case 3:
					return 1800;
				case 4:
					return 10'0000;
			}
		}
			
		if (ai){
			switch(ai){
				case 1:
					return 35;
				case 2:
					return 800;
				case 3:
					return 1'5000;
				case 4:
					return 80'0000;
			}
		}
		
		return -1;
	}
	
	inline void scan_horizontal() noexcept {		
		for (size_t r = 0;r < SIZE;++r){
			for (size_t c = 0;c < ELEVEN;++c){
				int human{}, ai{};
				size_t k = c;

				while(k < c + 5){
					if (chess.is_equal(r, k, ch_human))
						++human;
					else if (chess.is_equal(r, k, ch_ai))
						++ai;
					++k;
				}
				
				long score = evaluate(human, ai);
				
				for (k = c;k < c + 5;++k){
					if (chess.is_empty(r,k))
						scores[r][k] += score;
				}
			}
		}
	}
	
	inline void scan_vertical() noexcept {
		for (size_t r = 0;r < ELEVEN;++r){
			for (size_t c = 0;c < SIZE;++c){
				int human{}, ai{};
				size_t k = r;

				while(k < r + 5){
					if (chess.is_equal(k, c, ch_human))
						++human;
					else if (chess.is_equal(k, c, ch_ai))
						++ai;
					++k;
				}
				
				long score = evaluate(human, ai);
				
				for (k = r;k < r + 5;++k)
					if (chess.is_empty(k, c))
						scores[k][c] += score;
			}
		}
	}
	
	inline void scan_major_diagonal() noexcept {
		for (int r = 0;r < ELEVEN;++r){
			for (int c = 0;c < ELEVEN;++c){
				int human{}, ai{};
				size_t k = r;
				size_t m = c;
				
				while(k < r + 5){
					if (chess.is_equal(k, m, ch_human))
						++human;
					else if (chess.is_equal(k, m, ch_ai))
						++ai;
					
					++k;
					++m;
				}
				
				long score = evaluate(human, ai);
				
				for (k = r, m = c;k < r + 5;++k, ++m)
					if (chess.is_empty(k, m))
						scores[k][m] += score;
			}
		}
	}
	
	inline void scan_secondary_diagonal() noexcept {
		for (int r = 0;r < ELEVEN;++r){
			for (int c = FOUR;c < SIZE;++c){
				int human{}, ai{};
				size_t k = r;
				size_t m = c;
				
				while(k < r + 5){
					if (chess.is_equal(k, m, ch_human))
						++human;
					else if (chess.is_equal(k, m, ch_ai))
						++ai;
					
					++k;
					--m;
				}
				
				long score = evaluate(human, ai);
				
				for (k = r, m = c;k < r + 5;++k, --m)
					if (chess.is_empty(k, m))
						scores[k][m] += score;
			}
		}
	}
public:
	AI(Chess &chess_): chess{ chess_ } {}
	
	~AI(){}
	
	inline pair<size_t, size_t> generate_pos() noexcept {		
		scan_horizontal();
		scan_vertical();
		scan_major_diagonal();
		scan_secondary_diagonal();
		
		size_t r_max{}, c_max{};
		long score_max{};
		
		for (size_t r = 0;r < SIZE;++r){
			for (size_t c = 0;c < SIZE;++c){
				if (scores[r][c] > score_max){
					r_max = r;
					c_max = c;
					score_max = scores[r][c];
				}
			}
		}
		
		scores = {};             // every time we get a new position, we clear our scores for next use.
		return make_pair(r_max, c_max);
	}
};

class Manager{
	Chess chess;
	AI    ai;
	bool  human_first;
	stack<pair<size_t, size_t>> human_pos;
	stack<pair<size_t, size_t>> ai_pos;
	
	void go_back(){
		if (human_pos.empty() || ai_pos.empty())
			return;
		
		auto [r1, c1] = human_pos.top(); 
		human_pos.pop();
		chess.reset(r1, c1);
		
		auto[r2, c2] = ai_pos.top();
		ai_pos.pop();
		chess.reset(r2, c2);
		
		platform_specific::clear_screen();
		chess.print();
		
		if (!ai_pos.empty()){
			auto [r3, c3] = ai_pos.top();
			cout << "\nAI : [" << static_cast<char>(r3+'a') << "," << static_cast<char>(c3+'a') << "]\n";
		}
	}
	
	void do_ai_first(){
		chess.pos(7, 7, ch_ai);         // if ai first, go (h, h).
		platform_specific::clear_screen();
		chess.print();
		
		cout << "\nAI : [h,h]\n";		
	}
	
	bool do_human(){
		string input;
		
		for (;;){
			cout << "\nYou turn : ";
			getline(cin, input);
			
			if (input == "back"){
				go_back();
				continue;
			}
			
			if (chess.pos(input[0]-'a', input[1]-'a', ch_human)){
				human_pos.emplace(input[0]-'a', input[1]-'a');
				break;
			}
		}
		
		platform_specific::clear_screen();
		chess.print();
		
		return chess.check_player(input[0]-'a', input[1]-'a');
	}
	
	bool do_ai(){
		auto [r, c] = ai.generate_pos();
		ai_pos.emplace(r, c);
		chess.pos(r, c, ch_ai);
	
		platform_specific::clear_screen();
		chess.print();
		cout << "\nAI : [" << static_cast<char>(r+'a') << "," << static_cast<char>(c+'a') << "]\n";
	
		return chess.check_player(r, c);
	}
	
	void play_chess(){
		for (;;){
			if (do_human()){
				cout << "\nYou win.\n";
				break;
			}
			
			if (do_ai()){
				cout << "\nAI win.\n";
				break;
			}
		}
	}
public:
	Manager(bool human_first_ = true)
		: ai{ chess }, human_first{ human_first_ }
	{		
		chess.print();	
	}
	
	void start(){
		if (human_first){
			play_chess();
		}
		else{
			do_ai_first();
			play_chess();
		}
	}
};

int main(){
	Manager man{ false };
	man.start();
	platform_specific::pause();
}
