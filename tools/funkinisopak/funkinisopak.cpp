#include <tinyxml2.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#define FUNKISO_VERSION "1.0"

//Document globals
std::string xml_base;
std::string pc_directory;
tinyxml2::XMLDocument document;

//Document helpers
bool document_fail;

static std::string Document_ReadString(tinyxml2::XMLElement *element, std::string key)
{
	const char *result;
	if (element == nullptr || element->QueryStringAttribute(key.c_str(), &result) != tinyxml2::XML_SUCCESS)
	{
		document_fail = true;
		return "";
	}
	return std::string(result);
}

static std::string Document_ReadStringDef(tinyxml2::XMLElement *element, std::string key, std::string def)
{
	document_fail = false;
	std::string result = Document_ReadString(element, key);
	if (document_fail)
		return def;
	return result;
}

//Iso classes
class IsoProject
{
	private:
		//XML element
		tinyxml2::XMLElement *iso_project = nullptr;
		
	public:
		//Project info
		std::string image_name, cue_sheet;
		
	public:
		//Constructor and destructor
		IsoProject()
		{
			
		}
		
		~IsoProject()
		{
			
		}
		
		//Interface
		bool Read(tinyxml2::XMLElement *_iso_project)
		{
			//Use given element
			if ((iso_project = _iso_project) == nullptr)
			{
				std::cout << "Can't read IsoProject from null XML element" << std::endl;
				return true;
			}
			
			//Get output files
			document_fail = false;
			image_name = Document_ReadString(iso_project, "image_name");
			cue_sheet = Document_ReadString(iso_project, "cue_sheet");
			if (document_fail)
			{
				std::cout << "Failed to get image_name and cue_sheet from iso_project" << std::endl;
				return true;
			}
			
			std::cout << "Building ISO Image: " << image_name << " + " << cue_sheet << std::endl;
			
			//Print information
			std::cout << "  Track #1 data:" << std::endl;
			
			tinyxml2::XMLElement *identifiers = iso_project->FirstChildElement("identifiers");
			
			std::string system      = Document_ReadStringDef(identifiers, "system",      "PLAYSTATION");
			std::string application = Document_ReadStringDef(identifiers, "applicatios", "PLAYSTATION");
			std::string volume      = Document_ReadStringDef(identifiers, "volume",      "PLAYSTATION");
			std::string publisher   = Document_ReadStringDef(identifiers, "publisher",   "");
			std::string copyright   = Document_ReadStringDef(identifiers, "copyright",   "");
			
			tinyxml2::XMLElement *license = iso_project->FirstChildElement("license");
			std::string license_file = Document_ReadStringDef(license, "file", "");
			
			std::cout << "    Identifiers:" << std::endl;
			std::cout << "      System       : " << system << std::endl;
			std::cout << "      Application  : " << application << std::endl;
			std::cout << "      Volume       : " << volume << std::endl;
			std::cout << "      Publisher    : " << publisher << std::endl;
			std::cout << "      Copyright    : " << copyright << std::endl;
			std::cout << std::endl;
			std::cout << "    License file: " << license_file << std::endl;
			std::cout << std::endl;
			
			//Parse directory tree
			
			
			//Write cue sheet
			std::ofstream cue_file(xml_base + cue_sheet);
			if (cue_file.fail())
			{
				std::cout << "Failed to open " << cue_sheet << std::endl;
				return true;
			}
			
			cue_file << "FILE \"" << image_name << "\" BINARY" << '\n';
			cue_file << "  TRACK 00 MODE2/2352" << '\n';
			cue_file << "    INDEX 01 00:00:00" << '\n';
			
			return false;
		}
};

//Entry point
int main(int argc, char *argv[])
{
	//Print executable info
	std::cout << "funkinisopak " FUNKISO_VERSION " - PlayStation ISO Image Maker" << std::endl;
	std::cout << "2021 - 2021 Studio Cucky (CuckyDev)" << std::endl << std::endl;
	
	//Check for XML
	if (argc < 2)
	{
		std::cout << "usage: funkinisopak funkin.xml [optional: output directory (for pc)]" << std::endl;
		return 1;
	}
	
	//Get XML base path
	std::string xml_path = std::string(argv[1]);
	
	size_t xml_base_e = xml_path.find_last_of("/\\");
	if (xml_base_e != std::string::npos)
		xml_base = xml_path.substr(0, xml_base_e + 1);
	else
		xml_base = "./";
	
	//Get PC directory
	if (argc >= 3)
	{
		pc_directory = std::string(argv[2]);
		if (pc_directory.empty()) //Avoid writing to root
			pc_directory = "./";
		else if (pc_directory.back() != '/' && pc_directory.back() != '\\')
			pc_directory.append("/");
	}
	
	//Open XML document
	if (document.LoadFile(xml_path.c_str()) != tinyxml2::XML_SUCCESS)
	{
		switch (document.ErrorID())
		{
			case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
			case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
			case tinyxml2::XML_ERROR_FILE_READ_ERROR:
				std::cout << "Failed to open " << xml_path << std::endl;
				break;
			default:
				std::cout << document.ErrorName() << " on line " << document.ErrorLineNum() << std::endl;
				break;
		}
		return 1;
	}
	
	//Read ISO project
	for (tinyxml2::XMLElement *iso_project = document.FirstChildElement("iso_project"); iso_project != nullptr; iso_project = iso_project->NextSiblingElement("iso_project"))
	{
		IsoProject project;
		if (project.Read(iso_project))
			return 1;
	}
	
	return 0;
}