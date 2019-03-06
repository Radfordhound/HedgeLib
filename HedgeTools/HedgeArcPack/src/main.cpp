#include "types.h"
#include <Archives/ArchiveBase.h>
#include <Archives/LWArchive.h>
#include <filesystem>
#include <cstdlib>
#include <iostream>

using namespace HedgeLib::Archives;
using namespace HedgeArcPack;

int main(int argc, char* argv[])
{
	std::cout << "Extracting..." << std::endl;

	// TODO: Actual arguments
	std::filesystem::path filePath = argv[1]; 
	auto arc = GetArchive(filePath);
	arc->Load(filePath);
	arc->Extract("out");

	auto splitList = arc->GetSplitList(filePath);
	for (auto& splitPath : splitList)
	{
		arc->Load(splitPath);
		arc->Extract("out");
	}

	std::cout << "Done!" << std::endl;
	return EXIT_SUCCESS;
}
