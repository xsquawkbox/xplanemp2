/*
 * Copyright (c) 2005, Ben Supnik and Chris Serio.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <cstdio>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <functional>
#include <cctype>
#include <string>

#include <XPLMPlugin.h>
#include <XPLMUtilities.h>

#include "XPMPMultiplayer.h"
#include "CSLLibrary.h"
#include "XStringUtils.h"
#include "XUtils.h"
#include "obj8/Obj8CSL.h"
#include "legacycsl/LegacyCSL.h"

using namespace std;
using namespace xpmp;

#if APL
#include "AplFSUtil.h"
#endif

// Set this to 1 to get TONS of diagnostics on what the lib is doing.
#define		DEBUG_CSL_LOADING 0

// Set this to 1 to cause AIRLINE and LIVERY to create ICAO codes automatically
#define		USE_DEFAULTING 0

enum
{
	pass_Depend, pass_Load, pass_Count
};

/************************************************************************
 * UTILITY ROUTINES
 ************************************************************************/

static void
MakePartialPathNativeObj(string &io_str)
{
	//	char sep = *XPLMGetDirectorySeparator();
	for (size_t i = 0; i < io_str.size(); ++i) {
		if (io_str[i] == '/' || io_str[i] == ':' || io_str[i] == '\\') {
			io_str[i] = '/';
		}
	}
}

static bool
DoPackageSub(std::string &ioPath)
{
	for (auto i = gPackages.begin(); i != gPackages.end(); ++i) {
		if (strncmp(i->name.c_str(), ioPath.c_str(), i->name.size()) == 0) {
			ioPath.erase(0, i->name.size());
			ioPath.insert(0, i->path);
			return true;
		}
	}
	return false;
}

/************************************************************************
 * CSL LOADING
 ************************************************************************/

static bool
ParseExportCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: EXPORT_NAME command requires 1 argument.\n";
		return false;
	}

	auto p = std::find_if(
		gPackages.begin(), gPackages.end(), [&tokens](CSLPackage_t p) { return p.name == tokens[1]; });
	if (p == gPackages.end()) {
		package.path = path;
		package.name = tokens[1];
		return true;
	} else {
		XPLMDump(path, lineNum, line)
			<< XPMP_CLIENT_NAME " WARNING: Package name "
			<< tokens[1].c_str()
			<< " already in use by "
			<< p->path.c_str()
			<< " reqested by use by "
			<< path.c_str()
			<< "'\n";
		return false;
	}
}

static bool
ParseDependencyCommand(
	const std::vector<std::string> &tokens,
	CSLPackage_t &/*package*/,
	const string &path,
	int lineNum,
	const string &line)
{
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: DEPENDENCY command needs 1 argument.\n";
		return false;
	}

	if (std::count_if(gPackages.begin(), gPackages.end(), [&tokens](CSLPackage_t p) { return p.name == tokens[1]; }) ==
		0) {
		XPLMDump(path, lineNum, line)
			<< XPMP_CLIENT_NAME " WARNING: required package "
			<< tokens[1]
			<< " not found. Aborting processing of this package.\n";
		return false;
	}

	return true;
}

static bool
ParseObjectCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	std::vector<std::string> dupTokens = tokens;
	BreakStringPvt(line.c_str(), dupTokens, 2, " \t\r\n");
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: OBJECT command takes 1 argument.\n";
		return false;
	}
	std::string relativePath(tokens[1]);
	MakePartialPathNativeObj(relativePath);
	std::string fullPath(relativePath);
	if (!DoPackageSub(fullPath)) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: package not found.\n";
		return false;
	}

	std::vector<std::string> dirNames;
	BreakStringPvt(relativePath.c_str(), dirNames, 0, "/");
	// Replace the first one being the package name with the package root dir
	std::string packageRootDir = package.path.substr(package.path.find_last_of('/') + 1);
	dirNames[0] = packageRootDir;
	// Remove the last one being the obj itself
	string objFileName = dirNames.back();
	dirNames.pop_back();

	// Remove *.obj extension
	objFileName.erase(objFileName.find_last_of('.'));

	LegacyCSL *csl = new LegacyCSL(dirNames, objFileName, fullPath, OBJ_DefaultModel(fullPath));
	package.planes.push_back(csl);
#if DEBUG_CSL_LOADING
	XPLMDebugString("      Got Object: ");
	XPLMDebugString(fullPath.c_str());
	XPLMDebugString("\n");
#endif

	return true;
}

static bool
ParseTextureCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: TEXTURE command takes 1 argument.\n";
		return false;
	}

	// Load regular texture
	string relativeTexPath = tokens[1];
	MakePartialPathNativeObj(relativeTexPath);
	string absoluteTexPath(relativeTexPath);

	if (!DoPackageSub(absoluteTexPath)) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: package not found.\n";
		return false;
	}

	string textureFilename = absoluteTexPath;
	// Remove directory if present.
	textureFilename.erase(0, textureFilename.find_last_of('/') + 1);
	// Remove extension if present.
	textureFilename.erase(textureFilename.find_last_of('.'));

	LegacyCSL *myCSL = dynamic_cast<LegacyCSL *>(package.planes.back());
	myCSL->setTexture(textureFilename, absoluteTexPath, OBJ_GetLitTextureByTexture(absoluteTexPath));

#if DEBUG_CSL_LOADING
	XPLMDebugString("      Got texture: ");
	XPLMDebugString(absoluteTexPath.c_str());
	XPLMDebugString("\n");
#endif

	return true;
}

static bool
ParseObj8AircraftCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// OBJ8_AIRCRAFT <path>
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: OBJ8_AIRCRAFT command takes 1 argument.\n";
	}

	auto csl = new Obj8CSL({package.path.substr(package.path.find_last_of('/') + 1)}, tokens[1]);
	package.planes.push_back(csl);

#if DEBUG_CSL_LOADING
	XPLMDebugString("      Got OBJ8 Airplane: ");
	XPLMDebugString(tokens[1].c_str());
	XPLMDebugString("\n");
#endif
	return true;
}

static bool
ParseObj8Command(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// OBJ8 <group> <animate YES|NO> <filename>
	if (tokens.size() != 4) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: OBJ8 command takes 3 arguments.\n";
	}

	auto *myCSL = dynamic_cast<Obj8CSL *>(package.planes.back());
	// err - obj8 record at stupid place in file
	if (package.planes.empty() || myCSL == nullptr) {
		return false;
	}

	Obj8DrawType dt;
	if (tokens[1] == "LIGHTS") {
		dt = Obj8DrawType::LightsOnly;
	} else {
		if (tokens[1] == "LOW_LOD") {
			dt = Obj8DrawType::LowLevelOfDetail;
		} else {
			if (tokens[1] == "SOLID") {
				dt = Obj8DrawType::Solid;
			} else {
				// err crap enum
			}
		}
	}


#if 0
	// dataref costs are pretty negligable with the instancing API - obj8's are now always animated
	bool needs_animation = false;
	if (tokens[2] == "YES") {
		needs_animation = true;
	} else {
		if (tokens[2] == "NO") {
			needs_animation = false;
		} else {
			// crap flag
		}
	}
#endif

	string relativePath = tokens[3];
	MakePartialPathNativeObj(relativePath);
	string absolutePath(relativePath);
	if (!DoPackageSub(absolutePath)) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: package not found.\n";
		return false;
	}

	char xsystem[1024];
	XPLMGetSystemPath(xsystem);
#if APL
	if (XPLMIsFeatureEnabled("XPLM_USE_NATIVE_PATHS") == 0)
		HFS2PosixPath(xsystem, xsystem, 1024);
#endif

	size_t sys_len = strlen(xsystem);
	if (absolutePath.size() > sys_len) {
		absolutePath.erase(absolutePath.begin(), absolutePath.begin() + sys_len);
	} else {
		// should probaby freak out here.
	}

	Obj8Attachment att(absolutePath, dt);

	myCSL->addAttachment(std::move(att));

	return true;
}

static bool
ParseVertOffsetCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// VERT_OFFSET
	// this is the csl-model vertical offset for accurately putting planes onto the ground.
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: VERT_OFFSET command takes 1 argument.\n";
		return false;
	}
	package.planes.back()->setVerticalOffset(VerticalOffsetSource::Model, stof(tokens[1].c_str()));
	return true;
}

static bool
ParseHasGearCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// HASGEAR YES|NO
	if (tokens.size() != 2 || (tokens[1] != "YES" && tokens[1] != "NO")) {
		XPLMDump(path, lineNum, line)
			<< XPMP_CLIENT_NAME " WARNING: HASGEAR takes one argument that must be YES or NO.\n";
		return false;
	}

	if (tokens[1] == "YES") {
		package.planes.back()->setMovingGear(true);
		return true;
	} else {
		if (tokens[1] == "NO") {
			package.planes.back()->setMovingGear(false);
			return true;
		} else {
			XPLMDump(path, lineNum, line)
				<< XPMP_CLIENT_NAME " WARNING: HASGEAR must have a YES or NO argument, but we got "
				<< tokens[1]
				<< ".\n";
			return false;
		}
	}
}

static bool
ParseIcaoCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// ICAO <code>
	if (tokens.size() != 2) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: ICAO command takes 1 argument.\n";
		return false;
	}

	std::string icao = tokens[1];
	package.planes.back()->setICAO(icao);
	std::string group = gGroupings[icao];
	if (package.matches[match_icao].count(icao) == 0) {
		package.matches[match_icao][icao] = static_cast<int>(package.planes.size()) - 1;
	}
	if (!group.empty()) {
		if (package.matches[match_group].count(group) == 0) {
			package.matches[match_group][group] = static_cast<int>(package.planes.size()) - 1;
		}
	}

	return true;
}

static bool
ParseAirlineCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// AIRLINE <code> <airline>
	if (tokens.size() != 3) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: AIRLINE command takes two arguments.\n";
		return false;
	}


	std::string icao = tokens[1];
	std::string airline = tokens[2];
	package.planes.back()->setAirline(icao, airline);
	std::string group = gGroupings[icao];
	if (package.matches[match_icao_airline].count(icao + " " + airline) == 0) {
		package.matches[match_icao_airline][icao + " " + airline] = static_cast<int>(package.planes.size()) - 1;
	}
#if USE_DEFAULTING
	if (package.matches[match_icao		].count(icao				) == 0)
		package.matches[match_icao		]      [icao				] = package.planes.size() - 1;
#endif
	if (!group.empty()) {
#if USE_DEFAULTING
		if (package.matches[match_group	     ].count(group				  ) == 0)
			package.matches[match_group	     ]		[group				  ] = package.planes.size() - 1;
#endif
		if (package.matches[match_group_airline].count(group + " " + airline) == 0) {
			package.matches[match_group_airline][group + " " + airline] = static_cast<int>(package.planes.size()) - 1;
		}
	}

	return true;
}

static bool
ParseLiveryCommand(
	const std::vector<std::string> &tokens, CSLPackage_t &package, const string &path, int lineNum, const string &line)
{
	// LIVERY <code> <airline> <livery>
	if (tokens.size() != 4) {
		XPLMDump(path, lineNum, line) << XPMP_CLIENT_NAME " WARNING: LIVERY command takes two arguments.\n";
		return false;
	}

	std::string icao = tokens[1];
	std::string airline = tokens[2];
	std::string livery = tokens[3];
	package.planes.back()->setLivery(icao, airline, livery);
	std::string group = gGroupings[icao];
#if USE_DEFAULTING
	if (package.matches[match_icao				].count(icao							   ) == 0)
		package.matches[match_icao				]	   [icao							   ] = package.planes.size() - 1;
	if (package.matches[match_icao				].count(icao							   ) == 0)
		package.matches[match_icao_airline 		]	   [icao + " " + airline			   ] = package.planes.size() - 1;
#endif
	if (package.matches[match_icao_airline_livery].count(icao + " " + airline + " " + livery) == 0) {
		package.matches[match_icao_airline_livery][icao + " " + airline + " " + livery] =
			static_cast<int>(package.planes.size()) - 1;
	}
	if (package.matches[match_icao_livery].count(icao + " " + livery) == 0) {
		package.matches[match_icao_livery][icao + " " + livery] =
			static_cast<int>(package.planes.size()) - 1;
	}
	if (!group.empty()) {
#if USE_DEFAULTING
		if (package.matches[match_group		 		 ].count(group							     ) == 0)
			package.matches[match_group		 		 ]		[group							     ] = package.planes.size() - 1;
		if (package.matches[match_group_airline		 ].count(group + " " + airline			     ) == 0)
			package.matches[match_group_airline		 ]		[group + " " + airline			     ] = package.planes.size() - 1;
#endif
		if (package.matches[match_group_airline_livery].count(group + " " + airline + " " + livery) == 0) {
			package.matches[match_group_airline_livery][group + " " + airline + " " + livery] =
				static_cast<int>(package.planes.size()) - 1;
		}
		if (package.matches[match_group_livery].count(group + " " + livery) == 0) {
			package.matches[match_group_livery][group + " " + livery] =
				static_cast<int>(package.planes.size()) - 1;
		}
	}

	return true;
}

static bool
ParseDummyCommand(
	const std::vector<std::string> & /* tokens */,
	CSLPackage_t & /* package */,
	const string & /* path */,
	int /*lineNum*/,
	const string & /*line*/)
{
	return true;
}

static std::string
GetFileContent(const std::string &filename)
{
	std::string content;
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in) {
		content = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	}
	return content;
}

static CSLPackage_t
ParsePackageHeader(const string &path, const string &content)
{
	using command = std::function<bool(
		const std::vector<std::string> &, CSLPackage_t &, const string &, int, const string &)>;

	static const std::map<std::string, command> commands{{"EXPORT_NAME", &ParseExportCommand}};

	CSLPackage_t package;
	stringstream sin(content);
	if (!sin.good()) {
		return package;
	}

	std::string line;
	int lineNum = 0;

	while (std::getline(sin, line)) {
		++lineNum;
		auto tokens = tokenize(line, " \t\r\n");
		if (!tokens.empty()) {
			auto it = commands.find(tokens[0]);
			if (it != commands.end()) {
				bool result = it->second(tokens, package, path, lineNum, line);
				// Stop loop once we found EXPORT command
				if (result) {
					break;
				}
			}
		}
	}

	return package;
}


static void
ParseFullPackage(const std::string &content, CSLPackage_t &package)
{
	using command = std::function<bool(
		const std::vector<std::string> &, CSLPackage_t &, const string &, int, const string &)>;

	static const std::map<std::string, command> commands {
		{"EXPORT_NAME", &ParseDummyCommand},
		{"DEPENDENCY", &ParseDependencyCommand},
		{"OBJECT", &ParseObjectCommand},
		{"TEXTURE", &ParseTextureCommand},
		{"OBJ8_AIRCRAFT", &ParseObj8AircraftCommand},
		{"OBJ8", &ParseObj8Command},
		{"VERT_OFFSET", &ParseVertOffsetCommand},
		{"HASGEAR", &ParseHasGearCommand},
		{"ICAO", &ParseIcaoCommand},
		{"AIRLINE", &ParseAirlineCommand},
		{"LIVERY", &ParseLiveryCommand},
	};

	stringstream sin(content);
	if (!sin.good()) {
		return;
	} // exit if file not found

	std::string packageFilePath(package.path);
	packageFilePath += "/";
	packageFilePath += "xsb_aircraft.txt";

	std::string line;
	int lineNum = 0;
	while (std::getline(sin, line)) {
		++lineNum;
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		auto tokens = tokenize(line, " \t\r\n");
		if (!tokens.empty()) {
			auto it = commands.find(tokens[0]);
			if (it != commands.end()) {
				it->second(tokens, package, packageFilePath, lineNum, line);
			} else {
				XPLMDump(packageFilePath, lineNum, line);
			}
		}
	}
}

static bool
isPackageAlreadyLoaded(const std::string &packagePath)
{
	bool alreadyLoaded = false;
	for (const auto &package : gPackages) {
		if (package.path == packagePath) {
			alreadyLoaded = true;
			break;
		}
	}
	return alreadyLoaded;
}

bool
CSL_LoadData(const char *inRelated, const char *inDoc8643)
{
	bool ok = true;

	// read the list of aircraft codes
	FILE *aircraft_fi = fopen(inDoc8643, "r");

	if (aircraft_fi) {
		char buf[1024];
		while (fgets_multiplatform(buf, sizeof(buf), aircraft_fi)) {
			vector<string> tokens;
			BreakStringPvt(buf, tokens, 0, "\t\r\n");

			// Sample line. Fields are separated by tabs
			// ABHCO	SA-342 Gazelle 	GAZL	H1T	-

			if (tokens.size() < 5) {
				continue;
			}
			CSLAircraftCode_t entry;
			entry.icao = tokens[2];
			entry.equip = tokens[3];
			entry.category = tokens[4][0];

			gAircraftCodes[entry.icao] = entry;
		}
		fclose(aircraft_fi);
	} else {
		XPLMDump() << XPMP_CLIENT_NAME " WARNING: could not open ICAO document 8643 at " << inDoc8643 << "\n";
		ok = false;
	}

	// next, grab the related.txt file.
	FILE *related_fi = fopen(inRelated, "r");
	if (related_fi) {
		char buf[1024];
		while (fgets_multiplatform(buf, sizeof(buf), related_fi)) {
			if (buf[0] != ';') {
				vector<string> tokens;
				BreakStringPvt(buf, tokens, 0, " \t\r\n");
				string group;
				for (size_t n = 0; n < tokens.size(); ++n) {
					if (n != 0) {
						group += " ";
					}
					group += tokens[n];
				}
				for (const auto &tok: tokens) {
					gGroupings[tok] = group;
				}
			}
		}
		fclose(related_fi);
	} else {
		XPLMDump() << XPMP_CLIENT_NAME " WARNING: could not open related.txt at " << inRelated << "\n";
		ok = false;
	}

	return ok;
}

// This routine loads the related.txt file and also all packages.
bool
CSL_LoadCSL(const char *inFolderPath)
{
	bool ok = true;

	// Iterate through all directories using the XPLM and load them.
	char *name_buf = (char *) malloc(16384);
	char **index_buf = (char **) malloc(65536);
	int total, ret;

	char folder[1024];

#if APL
	if (XPLMIsFeatureEnabled("XPLM_USE_NATIVE_PATHS") == 0) {
		Posix2HFSPath(inFolderPath, folder, sizeof(folder));
	} else {
		strcpy(folder, inFolderPath);
	}
#else
	strcpy(folder, inFolderPath);
#endif
	XPLMGetDirectoryContents(folder, 0, name_buf, 16384, index_buf, 65536 / sizeof(char *), &total, &ret);

	vector<string> packageDirs;
	for (int r = 0; r < ret; ++r) {
#if APL
		if (index_buf[r][0] == '.')
			continue;
#endif
		char *foo = index_buf[r];
		string path(inFolderPath);
		path += "/";//XPLMGetDirectorySeparator();
		path += foo;
		packageDirs.push_back(path);
	}
	free(name_buf);
	free(index_buf);

	vector<CSLPackage_t> packages;

	// First read all headers. This is required to resolve the DEPENDENCIES
	for (const auto &packagePath : packageDirs) {
		std::string packageFile(packagePath);
		packageFile += "/"; //XPLMGetDirectorySeparator();
		packageFile += "xsb_aircraft.txt";

		// Continue if file does not exist or package was already loaded
		if (!DoesFileExist(packageFile) || isPackageAlreadyLoaded(packagePath)) {
			continue;
		}

		XPLMDump() << XPMP_CLIENT_NAME ": Loading package: " << packageFile << "\n";
		std::string packageContent = GetFileContent(packageFile);
		auto package = ParsePackageHeader(packagePath, packageContent);
		if (package.hasValidHeader()) {
			packages.push_back(package);
		}
	}

	if (!packages.empty()) {
		// iterator points to the first inserted package
		auto iterator = gPackages.insert(gPackages.end(), packages.begin(), packages.end());

		// Now we do a full run
		for (; iterator != gPackages.end(); ++iterator) {
			auto &package = *iterator;
			std::string packageFile(package.path);
			packageFile += "/"; //XPLMGetDirectorySeparator();
			packageFile += "xsb_aircraft.txt";
			std::string packageContent = GetFileContent(packageFile);
			ParseFullPackage(packageContent, package);
		}
	}

#if 0
	::Microseconds((UnsignedWide*) &t2);
	double delta = (t2 - t1);
	delta /= 1000000.0;
	char buf[256];
	sprintf(buf,"CSL full load took: %lf\n", delta);
	XPLMDebugString(buf);
#endif
	return ok;
}

/************************************************************************
 * CSL MATCHING
 ************************************************************************/

// Here's the basic idea: there are six levels of matching we can get,
// from the best (direct match of ICAO, airline and livery) to the worst
// (match an airplane's ICAO group but not ICAO, no livery or airline).
// So we will make six passes from best to worst, trying to match.  For
// each pass we try each package in turn from highest to lowest priority.


// These structs tell us how to build the matching keys for a given pass.
static const int kUseICAO[] = {1, 1, 0, 0, 1, 1, 0, 0};
static const int kUseAirline[] = {1, 1, 1, 1, 0, 0, 0, 0};
static const int kUseLivery[] = {1, 0, 1, 0, 1, 0, 1, 0};

CSL *
CSL_MatchPlane(const PlaneType &type,int *match_quality, bool allow_default)
{
	string group;
	string key;

	map<string, string>::iterator group_iter = gGroupings.find(type.mICAO);
	if (group_iter != gGroupings.end()) {
		group = group_iter->second;
	}

	char buf[4096];

	if (gConfiguration.debug.modelMatching) {
		snprintf(
			buf,
			4096,
			XPMP_CLIENT_NAME " MATCH - %s - GROUP=%s\n",
			type.toLongString().c_str(),
			group.c_str());
		XPLMDebugString(buf);
	}

	// Now we go through our passes.
	for (int n = 0; n < match_count; ++n) {
		// Build up the right key for this pass.
		key = kUseICAO[n]?type.mICAO:group;
		if (!kUseICAO[n] && group.empty()) {
			if (gConfiguration.debug.modelMatching) {
				sprintf(buf, XPMP_CLIENT_NAME " MATCH -    Skipping %d Due nil Group\n", n);
				XPLMDebugString(buf);
			}
		}

		if (kUseAirline[n]) {
			if (type.mAirline.empty()) {
				if (gConfiguration.debug.modelMatching) {
					sprintf(buf, XPMP_CLIENT_NAME " MATCH -    Skipping %d Due Absent Airline\n", n);
					XPLMDebugString(buf);
				}
				continue;
			}
			key += " ";
			key += type.mAirline;
		}

		if (kUseLivery[n]) {
			if (type.mLivery.empty()) {
				if (gConfiguration.debug.modelMatching) {
					sprintf(buf, XPMP_CLIENT_NAME " MATCH -    Skipping %d Due Absent Livery\n", n);
					XPLMDebugString(buf);
				}
				continue;
			}
			key += " ";
			key += type.mLivery;
		}

		if (gConfiguration.debug.modelMatching) {
			sprintf(buf, XPMP_CLIENT_NAME " MATCH -    Group %d key %s\n", n, key.c_str());
			XPLMDebugString(buf);
		}

		// Now go through each group and see if we match.
		for (const auto &package: gPackages) {
			auto iter = package.matches[n].find(key);
			if (iter != package.matches[n].end()) {
				if (!package.planes[iter->second]->isUsable()) {
					if (gConfiguration.debug.modelMatching) {
						sprintf(
							buf,
							XPMP_CLIENT_NAME " MATCH - Skipping as not usable. Found: %s/%s/%s : %s\n",
							package.planes[iter->second]->getICAO().c_str(),
							package.planes[iter->second]->getAirline().c_str(),
							package.planes[iter->second]->getLivery().c_str(),
							package.planes[iter->second]->getModelName().c_str());
						XPLMDebugString(buf);
					}
					continue;
				}
				if (nullptr != match_quality) {
					*match_quality = n;
				}
				if (gConfiguration.debug.modelMatching) {
					sprintf(
						buf,
						XPMP_CLIENT_NAME " MATCH - Found: %s/%s/%s : %s\n",
						package.planes[iter->second]->getICAO().c_str(),
						package.planes[iter->second]->getAirline().c_str(),
						package.planes[iter->second]->getLivery().c_str(),
						package.planes[iter->second]->getModelName().c_str());
					XPLMDebugString(buf);
				}
				return package.planes[iter->second];
			}
		}
	}

	if (gConfiguration.debug.modelMatching) {
		XPLMDebugString(XPMP_CLIENT_NAME " MATCH - No match.\n");
	}
	if (NULL != match_quality) {
		*match_quality = -1;
	}

	// try the next step:
	// For each aircraft, we know the equipment type "L2T" and the WTC category.
	// try to find a model that has the same equipment type and WTC

	const auto model_it = gAircraftCodes.find(type.mICAO);
	if (model_it != gAircraftCodes.end()) {
		if (gConfiguration.debug.modelMatching) {
			XPLMDebugString(XPMP_CLIENT_NAME " MATCH/eqp-fallback - Looking for a ");
			switch (model_it->second.category) {
			case 'L':
				XPLMDebugString(" light ");
				break;
			case 'M':
				XPLMDebugString(" medium ");
				break;
			case 'H':
				XPLMDebugString(" heavy ");
				break;
			default:
				XPLMDebugString(" funny ");
				break;
			}
			XPLMDebugString(model_it->second.equip.c_str());
			XPLMDebugString(" aircraft\n");
		}

		// 1. match WTC, full configuration ("L2P")
		// 2. match WTC, #engines and enginetype ("2P")
		// 3. match WTC, #egines ("2")
		// 4. match WTC, enginetype ("P")
		// 5. match WTC
		for (int pass = 0; pass <= match_fallback_count; ++pass) {

			if (gConfiguration.debug.modelMatching) {
				switch (pass) {
				case 1:
					XPLMDebugString(XPMP_CLIENT_NAME " Match/eqp-fallback - matching WTC and configuration\n");
					break;
				case 2:
					XPLMDebugString(XPMP_CLIENT_NAME " Match/eqp-fallback - matching WTC, #engines and enginetype\n");
					break;
				case 3:
					XPLMDebugString(XPMP_CLIENT_NAME " Match/eqp-fallback - matching WTC, #engines\n");
					break;
				case 4:
					XPLMDebugString(XPMP_CLIENT_NAME " Match/eqp-fallback - matching WTC, enginetype\n");
					break;
				case 5:
					XPLMDebugString(XPMP_CLIENT_NAME " Match/eqp-fallback - matching WTC\n");
					break;
				}
			}


			for (const auto &package: gPackages) {
				// now we traverse all generic aircraft types in the package
				for (const auto &matchpair: package.matches[match_icao]) {
					if (package.planes[matchpair.second]->isUsable()) {
						// we have a candidate, lets see if it matches our criteria
						const auto m = gAircraftCodes.find(matchpair.first);
						if (m != gAircraftCodes.end()) {
							// category
							if (m->second.category != model_it->second.category) {
								continue;
							}
							switch (pass) {
							case match_fallback_wtc_fullconfig:	// perfect match of equipment.
								if (m->second.equip != model_it->second.equip)
									continue;
								break;
							case match_fallback_wtc_engines_enginetype:
								// this case will be caught by the enginetype case matching.
							case match_fallback_wtc_engines:
								if (m->second.equip[1] != model_it->second.equip[1])
									continue;
							case match_fallback_wtc_enginetype:
								if (m->second.equip.length() != 3) {
									continue;
								}
								if ((pass != match_fallback_wtc_engines) &&
									(m->second.equip[2] != model_it->second.equip[2])) {
									continue;
								}
							default:
								break;
							}
							// bingo
							if (gConfiguration.debug.modelMatching) {
								XPLMDebugString(XPMP_CLIENT_NAME " MATCH/eqp-fallback - found: ");
								XPLMDebugString(matchpair.first.c_str());
								XPLMDebugString("\n");
							}
							if (match_quality != nullptr) {
								*match_quality = match_count + pass;
							}
							return package.planes[matchpair.second];
						}
					}
				}
			}
		}
	}

	if (gConfiguration.debug.modelMatching) {
		XPLMDebugString(string("gAircraftCodes.find(" + type.mICAO + ") returned no match.\n").c_str());
	}

	if (type.compare(gDefaultPlane, Mask_ICAO)) {
		return nullptr;
	}
	if (!allow_default) {
		return nullptr;
	}
	int		defaultMatchQuality = 0;
	auto *defCSL = CSL_MatchPlane(gDefaultPlane, &defaultMatchQuality, false);
	if (match_quality != nullptr) {
		if (defaultMatchQuality > 0) {
			*match_quality = match_count + match_fallback_count + defaultMatchQuality;
		} else {
			*match_quality = -1;
		}
	}
	return defCSL;
}

void
CSL_Dump()
{
	// DIAGNOSTICS - print out everything we know.
	for (const auto &package: gPackages) {
		XPLMDump() << XPMP_CLIENT_NAME " CSL: Package " << package.name << "\n";
		for (size_t p = 0; p < package.planes.size(); ++p) {
			XPLMDump()
				<< XPMP_CLIENT_NAME " CSL:         Plane "
				<< p
				<< " = "
				<< package.planes[p]->getModelName()
				<< "\n";
		}
		for (int t = 0; t < match_count; ++t) {
			XPLMDump() << XPMP_CLIENT_NAME " CSL:           Table " << t << "\n";
			for (const auto &i: package.matches[t]) {
				XPLMDump() << XPMP_CLIENT_NAME " CSL:                " << i.first << " -> " << i.second << "\n";
			}
		}
	}
}
