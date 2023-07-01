#include <iostream>
#include <vector>
#include <chrono>
#include "USN.h"

using namespace std;
using namespace chrono;

int main() {              //  Running this code under admin rights.
	vector<searchEngine> vec;
	
	auto start = system_clock::now();

	try {
		vec.emplace_back(searchEngine{ L"C:" });    // using a thread-pool maybe better ? 
		vec.emplace_back(searchEngine{ L"D:" });
		vec.emplace_back(searchEngine{ L"E:" });
	}
	catch (const exception &e) {
		cerr << e.what() << endl;
		return -1;
	}                        

	auto end = system_clock::now();
	cout << "Initialize : " << duration_cast<milliseconds>(end - start).count() << " ms"<< endl;       // On my computer , it takes 1 minute in average .(700GB)

	wcin.imbue(locale(""));            // Don't forget this , otherwise you would get nothing.
	wstring user_input;
	for (;;) {
		std::cout << "search: ";
		getline(wcin,user_input);
		for (auto& e : vec) {
			e.search(user_input);           
		}
		cout << "-------------------------------\n";
	}
	
	system("pause");
}