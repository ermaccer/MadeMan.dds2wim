// dds2wim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dds.h"
#include <fstream>
#include <iostream>
#include <memory>


// just important stuff, lots of guessing going on

struct WIMHeader {
	int          header;  //WIMG for Windows Image | XIMG for XBOX Image
	int          fileSize;
	int          version; // seems to be 1
	int          type;   // body, weap, sk00, EXTR, hats, rlgs, lots of them actually
	                     // dds2wim supports replacing skins, pickups, props and weapons
	                     // right now it's using preset values, calculation is needed
	int          colorDepth; // seems to be 24
	int          unknown2[3];
	unsigned int size;      // doesn't support stuff like 64x128 apparently
	int          unknown3[2];
	char         dxtType[3];
};


std::streampos getSizeToEnd(std::ifstream& is)
{
	auto currentPosition = is.tellg();
	is.seekg(0, is.end);
	auto length = is.tellg() - currentPosition;
	is.seekg(currentPosition, is.beg);
	return length;
}

int main(int argc, char* argv[])
{

	if (argc != 3) {
		std::cout << "Usage dds2wim <input> <mode> \n Modes: \n weap - weapons \n body - characters \n ammo - pickups(mostly) \n extr - props \n" << std::endl;
		return 1;
	}


	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile) {
		std::cout << "ERROR: Could not open " << argv[1] << "!" << std::endl;
		return 1;
	}


	if (pFile)
	{
		DDSHeader dds;
		pFile.read((char*)&dds, sizeof(DDSHeader));
		// if it's not DDS bail out!
		if (!(dds.magic == ' SDD'))
		{
			std::cout << "ERROR: " << argv[1] << " is not a DDS image." << std::endl;
			return 1;
		}


		// write wim header!
		WIMHeader wim;
		wim.header = 'GMIW';
		wim.fileSize = (int)getSizeToEnd(pFile) + sizeof(WIMHeader);
		wim.version = 1;
		if (strcmp("weap", argv[2]) == 0)
		{
			wim.type = 'paew';
			wim.unknown2[1] = 0x0080008A;
			wim.unknown3[1] = 0x00040000;
		}
		if (strcmp("body", argv[2]) == 0)
		{
			wim.type = 'ydob';
			wim.unknown2[1] = 0x0200008A;
			wim.unknown3[1] = 0x00060000;
		}
		if (strcmp("ammo", argv[2]) == 0)
		{
			wim.type = 'omma';
			wim.unknown2[1] = 0x4000008A;
			wim.unknown3[1] = 0x00030000;
		}
		if (strcmp("extr", argv[2]) == 0)
		{
			wim.type = 'RTXE';
			wim.unknown2[1] = 0x8000008A;
			wim.unknown3[1] = 0x00030000;
		}
		wim.size = dds.dwHeight;
		wim.colorDepth = 24;
		wim.unknown2[0] = 0;
		wim.unknown2[2] = 0;
		wim.unknown3[0] = 0;
		wim.dxtType[0] = dds.ddspf.dwFourCC[3];
		wim.dxtType[1] = dds.ddspf.dwFourCC[2];
		wim.dxtType[2] = dds.ddspf.dwFourCC[1];
		wim.dxtType[3] = dds.ddspf.dwFourCC[0];
		// set after header to get dds data
		pFile.seekg(sizeof(DDSHeader), pFile.beg);

		// create output
		std::string output = argv[1];
		int length = output.length();
		output.replace(length - 4, 4, ".wim");
		std::ofstream oFile(output, std::ofstream::binary);

		// write header
		oFile.write((char*)&wim, sizeof(WIMHeader));
		auto dataSize = getSizeToEnd(pFile);
		auto dataBuff = std::make_unique<char[]>((long)dataSize);
		pFile.read(dataBuff.get(), dataSize);
		oFile.write(dataBuff.get(), dataSize);
		

	}

	return 0;
}

