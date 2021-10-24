/*
 * funkinpicopak by Regan "CuckyDev" Green
 * Packs Friday Night Funkin' json formatted charts (PICO CHART) into a binary file for the PSX port
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <unordered_set>

#include "json.hpp"
using json = nlohmann::json;

void WriteWord(std::ostream &out, uint16_t word)
{
	out.put(word >> 0);
	out.put(word >> 8);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cout << "usage: funkinchrpak in_tim out_chr" << std::endl;
		return 0;
	}
	
	//Read json
	std::string json_path = std::string(argv[2]) + ".json";
	std::ifstream i(json_path);
	if (!i.is_open())
	{
		std::cout << "Failed to open " << json_path << std::endl;
		return 1;
	}
	json j;
	i >> j;
	
	return 0;
}
