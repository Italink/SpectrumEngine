#include "AudioCapture.h"

#include "SpectrumEngine.h"

#include "SpectrumProvider.h"

#include <iostream>

int main() {
	auto it = SpectrumEngine::getInstance()->enumDevices();

	SpectrumEngine::getInstance()->start(it.front());
	SpectrumProvider provider(0, 40000, 80);
	std::vector<std::string> map(40, std::string(provider.getBarCount(), ' '));

	while (true) {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		//int columns = min(120,csbi.srWindow.Right - csbi.srWindow.Left) ;
		//if (map.front().size() != columns) {
		//	provider.setBarCount(columns);
		//	map.clear();
		//}
		if (map.size() != rows) {
			map.resize(rows, std::string(provider.getBarCount(), ' '));
		}

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
		std::vector<double> specData = provider.getBars();
		for (int i = 0; i < specData.size(); i++) {
			int fillCount = map.size() * specData[i];;
			for (int j = 0; j < fillCount; j++) {
				map[j][i] = '*';
			}
			for (int j = fillCount; j < map.size(); j++) {
				map[j][i] = ' ';
			}
		}
		for (int i = map.size() - 1; i >= 0; i--) {
			std::cout << map[i] << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}