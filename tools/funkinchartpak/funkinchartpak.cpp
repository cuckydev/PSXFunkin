/*
 * funkinchartpak by Regan "CuckyDev" Green
 * Packs Friday Night Funkin' json formatted charts into a binary file for the PSX port
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <unordered_set>

#include "json.hpp"
using json = nlohmann::json;

#define SECTION_FLAG_OPPFOCUS (1 << 15) //Focus on opponent
#define SECTION_FLAG_BPM_MASK 0x7FFF //1/24

struct Section
{
	uint16_t end;
	uint16_t flag = 0;
};

#define NOTE_FLAG_OPPONENT    (1 << 2) //Note is opponent's
#define NOTE_FLAG_SUSTAIN     (1 << 3) //Note is a sustain note
#define NOTE_FLAG_SUSTAIN_END (1 << 4) //Is either end of sustain
#define NOTE_FLAG_ALT_ANIM    (1 << 5) //Note plays alt animation
#define NOTE_FLAG_MINE        (1 << 6) //Note is a mine
#define NOTE_FLAG_HIT         (1 << 7) //Note has been hit

struct Note
{
	uint16_t pos; //1/12 steps
	uint8_t type, pad = 0;
};

uint16_t PosRound(double pos, double crochet)
{
	return (uint16_t)std::floor(pos / crochet + 0.5);
}

void WriteWord(std::ostream &out, uint16_t word)
{
	out.put(word >> 0);
	out.put(word >> 8);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "usage: funkinchartpak in_json" << std::endl;
		return 0;
	}
	
	//Read json
	std::ifstream i(argv[1]);
	if (!i.is_open())
	{
		std::cout << "Failed to open " << argv[1] << std::endl;
		return 1;
	}
	json j;
	i >> j;
	
	auto song_info = j["song"];
	
	double bpm = song_info["bpm"];
	double crochet = (60.0 / bpm) * 1000.0;
	double step_crochet = crochet / 4;
	
	double speed = song_info["speed"];
	
	std::cout << argv[1] << " speed: " << speed << " ini bpm: " << bpm << " step_crochet: " << step_crochet << std::endl;
	
	double milli_base = 0;
	uint16_t step_base = 0;
	
	std::vector<Section> sections;
	std::vector<Note> notes;
	
	uint16_t section_end = 0;
	int score = 0, dups = 0;
	std::unordered_set<uint32_t> note_fudge;
	for (auto &i : song_info["notes"]) //Iterate through sections
	{
		bool is_opponent = i["mustHitSection"] != true; //Note: swapped
		
		//Read section
		Section new_section;
		if (i["changeBPM"] == true)
		{
			//Update BPM (THIS IS HELL!)
			milli_base += step_crochet * (section_end - step_base);
			step_base = section_end;
			
			bpm = i["bpm"];
			crochet = (60.0 / bpm) * 1000.0;
			step_crochet = crochet / 4;
			
			std::cout << "chg bpm: " << bpm << " step_crochet: " << step_crochet << " milli_base: " << milli_base << " step_base: " << step_base << std::endl;
		}
		new_section.end = (section_end += 16) * 12; //(uint16_t)i["lengthInSteps"]) * 12; //I had to do this for compatibility
		new_section.flag = PosRound(bpm, 1.0 / 24.0) & SECTION_FLAG_BPM_MASK; 
		bool is_alt = i["altAnim"] == true;
		if (is_opponent)
			new_section.flag |= SECTION_FLAG_OPPFOCUS;
		sections.push_back(new_section);
		
		//Read notes
		for (auto &j : i["sectionNotes"])
		{
			//Push main note
			Note new_note;
			int sustain = (int)PosRound(j[2], step_crochet) - 1;
			new_note.pos = (step_base * 12) + PosRound(((double)j[0] - milli_base) * 12.0, step_crochet);
			new_note.type = (uint8_t)j[1] & (3 | NOTE_FLAG_OPPONENT);
			if (is_opponent)
				new_note.type ^= NOTE_FLAG_OPPONENT;
			if (j[3] == true)
				new_note.type |= NOTE_FLAG_ALT_ANIM;
			else if ((new_note.type & NOTE_FLAG_OPPONENT) && is_alt)
				new_note.type |= NOTE_FLAG_ALT_ANIM;
			if (sustain >= 0)
				new_note.type |= NOTE_FLAG_SUSTAIN_END;
			if (((uint8_t)j[1]) & 8)
				new_note.type |= NOTE_FLAG_MINE;
			
			if (note_fudge.count(*((uint32_t*)&new_note)))
			{
				dups += 1;
				continue;
			}
			note_fudge.insert(*((uint32_t*)&new_note));
				
			notes.push_back(new_note);
			if (!(new_note.type & NOTE_FLAG_OPPONENT))
				score += 350;
			
			//Push sustain notes
			for (int k = 0; k <= sustain; k++)
			{
				Note sus_note; //jerma
				sus_note.pos = new_note.pos + ((k + 1) * 12);
				sus_note.type = new_note.type | NOTE_FLAG_SUSTAIN;
				if (k != sustain)
					sus_note.type &= ~NOTE_FLAG_SUSTAIN_END;
				notes.push_back(sus_note);
			}
		}
	}
	std::cout << "max score: " << score << " dups excluded: " << dups << std::endl;
	
	//Sort notes
	std::sort(notes.begin(), notes.end(), [](Note a, Note b) {
		if (a.pos == b.pos)
			return (b.type & NOTE_FLAG_SUSTAIN) && !(a.type & NOTE_FLAG_SUSTAIN);
		else
			return a.pos < b.pos;
	});
	
	//Push dummy section and note
	Section dum_section;
	dum_section.end = 0xFFFF;
	dum_section.flag = sections[sections.size() - 1].flag;
	sections.push_back(dum_section);
	
	Note dum_note;
	dum_note.pos = 0xFFFF;
	dum_note.type = NOTE_FLAG_HIT;
	notes.push_back(dum_note);
	
	//Write to output
	std::ofstream out(std::string(argv[1]) + ".cht", std::ostream::binary);
	if (!out.is_open())
	{
		std::cout << "Failed to open " << argv[1] << ".cht" << std::endl;
		return 1;
	}
	
	//Write sections
	WriteWord(out, 2 + (sections.size() << 2));
	for (auto &i : sections)
	{
		WriteWord(out, i.end);
		WriteWord(out, i.flag);
	}
	
	//Write notes
	for (auto &i : notes)
	{
		WriteWord(out, i.pos);
		out.put(i.type);
		out.put(0);
	}
	return 0;
}
