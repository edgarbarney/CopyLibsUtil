#include <iostream>

#include <string>
#include <fstream>
#include <filesystem>

#include <map>
#include <vector>

#include "CopyLibsUtil.h"

#include "json/json.hpp"

// I'm not really a fan of using namespace

using json = nlohmann::json;

void ProcessPathString(std::string& tempDir, std::filesystem::path& fullPath, int argc, char* argv[])
{
	// If first characters are up dir identifier
	if (tempDir.substr(0, std::strlen(MI_DirIdentifier.beginning)) == MI_DirIdentifier.beginning)
	{
		size_t endPos = tempDir.find(MI_DirIdentifier.ending);
		if (endPos == std::string::npos)
			throw "Directory 'Up' directive did not close properly.";

		// Go x folders up
		size_t upDirective = tempDir.substr(std::strlen(MI_DirIdentifier.beginning), endPos - std::strlen(MI_DirIdentifier.beginning)).size();

		for (size_t i = 0; i < upDirective; i++)
		{
			fullPath = fullPath.parent_path();
		}

		// No need to calcualate exact size because the substr function is safe
		tempDir = tempDir.substr(endPos + 1, tempDir.size());
	}
	
	size_t pos = tempDir.find(MI_ArgIdentifier.beginning);

	while (pos != std::string::npos)
	{
		size_t endPos = tempDir.find(MI_ArgIdentifier.ending, pos + std::strlen(MI_ArgIdentifier.beginning));
		if (endPos == std::string::npos)
			throw "Directory 'Up' directive did not close properly.";

		int argindex = std::stoi(tempDir.substr(pos + std::strlen(MI_ArgIdentifier.beginning), endPos - (pos + std::strlen(MI_ArgIdentifier.beginning))));

		// Replace with args :O
		tempDir.replace(pos, endPos - pos + 1, argv[argindex]);

		pos = tempDir.find(MI_ArgIdentifier.beginning); //Search for others
	}


	if (std::filesystem::path(tempDir).is_relative())
	{
		fullPath += tempDir;
	}
	else //is absolute
	{
		fullPath = tempDir;
	}
	
}

int main(int argc, char* argv[])
{
	// std::filesystem::current_path(); Runs the "call" directory. We need the EXECUTABLE directory
	// You can uncomment this line and comment the other to prevent this
	// const std::filesystem::path currentDir = std::filesystem::current_path();
	const std::filesystem::path currentDir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path();
	
	std::cout << "\n\nFDreamer CopyLibsUtil - Started from : " << currentDir << "\n";

	std::vector<std::string> filelist;

	std::filesystem::path inputDir;
	std::filesystem::path outputDir;
	std::ifstream inputFile(std::filesystem::path(currentDir)+="/copylibs_filelist.json");

	try
	{
		json fullFile = json::parse(inputFile);

		json dirs = fullFile["Main"]["Dirs"]; // Directory Header
		json mainBranch = fullFile["Main"]["Data"]; //Direct to the main data.
		
		{
			std::string tempInputDir = dirs["From"];
			std::string tempOutputDir = dirs["To"];

			// Do we need these?
			std::filesystem::path fullInPath = currentDir;
			std::filesystem::path fullOutPath = currentDir;
			
			ProcessPathString(tempInputDir, fullInPath, argc, argv);
			ProcessPathString(tempOutputDir, fullOutPath, argc, argv);

			inputDir = fullInPath.make_preferred();
			outputDir = fullOutPath.make_preferred();
		}

		if (mainBranch.is_array()) // "Data" header must be an array
		{
			json* currentBranch = &mainBranch;
			// Go through args
			for (int i = 0; i < argc; i++)
			{
				for (auto& subBranchRef : *currentBranch)
				{
					auto* subBranch = &subBranchRef;

					if (subBranch->is_array())
					{
						subBranch = &subBranchRef[0]; // Set the only child of the array
					}

					if ((*subBranch)["type"].get<std::string>() == argv[i])
					{
						if ((*subBranch)["data"][0].is_object())
						{
							currentBranch = subBranch;
						}
						else
						{
							auto tempVec = (*subBranch)["data"].get<std::vector<std::string>>();
							filelist.insert(filelist.end(), tempVec.begin(), tempVec.end());
						}
						break;
					}
				}
			}
		}
	}
	catch (json::exception ex)
	{
		std::cout << "Json Exception! :\n" << ex.what() << std::endl;
		return 100;
	}
	catch (const char* ex)
	{
		std::cout << "Parse Exception!: \n" << ex << std::endl;

		return 200;
	}

	try
	{
		for (auto& tempstr : filelist)
		{
			//std::filesystem::copy(std::filesystem::path(inputDir) += tempstr, outputDir);
			auto tempInputPath = std::filesystem::path(inputDir);
			tempInputPath += tempstr;
			tempInputPath.make_preferred();
			std::cout << "\nCopying file...\nFrom : " << tempInputPath;
			std::cout << "To : " << outputDir;
			std::filesystem::copy_file(tempInputPath, outputDir.string() + tempInputPath.filename().string(), std::filesystem::copy_options::overwrite_existing);
		}
	}
	catch (std::filesystem::filesystem_error ex)
	{
		std::cout << "Filesystem Error!: \n" << ex.what() << std::endl;

		return 300;
	}

	std::cout << "\n\nFDreamer CopyLibsUtil - Task finished.\n\n" << std::endl;

	return 0;
}
