
#define NOMINMAX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <limits>
#include <algorithm>

#include "minigsf.h"
#include "PSFFile.h"
#include "cpath.h"

#ifdef WIN32
#include <direct.h>
#include <float.h>
#define getcwd _getcwd
#define chdir _chdir
#define isnan _isnan
#define strcasecmp _stricmp
#else
#include <unistd.h>
#endif

#define APP_NAME    "minigsf"
#define APP_VER     "[2019-02-04]"
#define APP_URL     "http://github.com/loveemu/minigsf"

#define GSF_PSF_VERSION         0x22
#define GSF_EXE_HEADER_SIZE     12

static void writeInt(uint8_t * buf, uint32_t value)
{
	buf[0] = value & 0xff;
	buf[1] = (value >> 8) & 0xff;
	buf[2] = (value >> 16) & 0xff;
	buf[3] = (value >> 24) & 0xff;
}

bool exe2gsf(const char * gsf_path, uint8_t * exe, uint32_t exe_size, std::map<std::string, std::string> tags)
{
	ZlibWriter zlib_exe(Z_BEST_COMPRESSION);
	zlib_exe.write(exe, exe_size);

	if (!PSFFile::save(gsf_path, GSF_PSF_VERSION, NULL, 0, zlib_exe, tags)) {
		return false;
	}

	return true;
}

bool make_minigsf(const char * gsf_path, uint32_t address, uint32_t size, uint32_t num, std::map<std::string, std::string>& tags)
{
	uint8_t exe[GSF_EXE_HEADER_SIZE + 256];
	memset(exe, 0, GSF_EXE_HEADER_SIZE + 256);

	// limit size
	if (size > 256) {
		return false;
	}

	// make exe
	uint32_t entrypoint = address & 0xFF000000;
	if (entrypoint == 0x9000000)
		entrypoint = 0x8000000;
	writeInt(&exe[0], entrypoint);
	writeInt(&exe[4], address);
	writeInt(&exe[8], size);
	writeInt(&exe[12], num);

	// write mini2sf file
	return exe2gsf(gsf_path, exe, GSF_EXE_HEADER_SIZE + size, tags);
}

static void usage(const char * progname)
{
	printf("%s %s\n", APP_NAME, APP_VER);
	printf("<%s>\n", APP_URL);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s (options) [Base name] [Offset] [Size] [Count]`\n", progname);
	printf("\n");

	printf("### Options\n");
	printf("\n");
	printf("`--help`\n");
	printf("  : Show help\n");
	printf("\n");
	printf("`--psfby`, `--gsfby` [name]\n");
	printf("  : Set creator name of GSF\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	long longval;
	char *endptr = NULL;

	char *psfby = NULL;

	int argi = 1;
	while (argi < argc && argv[argi][0] == '-') {
		if (strcmp(argv[argi], "--help") == 0) {
			usage(argv[0]);
			return EXIT_FAILURE;
		}
		else if (strcmp(argv[argi], "--psfby") == 0 || strcmp(argv[argi], "--gsfby") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			psfby = argv[argi + 1];
			argi++;
		}
		else {
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	int argnum = argc - argi;
	if (argnum != 4) {
		fprintf(stderr, "Error: Too few/more arguments\n");
		return EXIT_FAILURE;
	}

	char * gsf_basename = argv[argi];

	char libname[PATH_MAX];
	sprintf(libname, "%s.gsflib", gsf_basename);

	longval = strtol(argv[argi + 1], &endptr, 16);
	if (*endptr != '\0' || errno == ERANGE || longval < 0) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 1]);
		return EXIT_FAILURE;
	}
	uint32_t offset = (uint32_t)longval;
	if (!((offset >= 0x8000000 && offset <= 0x9ffffff) || (offset >= 0x2000000 && offset <= 0x203ffff))) {
		fprintf(stderr, "Error: Load offset 0x%08X is out of range\n", offset);
		fprintf(stderr, "       Valid ranges are 0x8000000..0x9FFFFFF and 0x2000000..0x203FFFF (for multiboot ROM)\n");
		return EXIT_FAILURE;
	}

	longval = strtol(argv[argi + 2], &endptr, 10);
	if (*endptr != '\0' || errno == ERANGE || longval < 0) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 2]);
		return EXIT_FAILURE;
	}
	size_t size = (size_t)longval;

	longval = strtol(argv[argi + 3], &endptr, 10);
	if (*endptr != '\0' || errno == ERANGE || longval < 0) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 3]);
		return EXIT_FAILURE;
	}
	uint32_t count = (uint32_t)longval;

	int num_error = 0;
	for (uint32_t num = 0; num < count; num++) {
		std::map<std::string, std::string> tags;
		tags["_lib"] = libname;

		if (psfby != NULL && strcmp(psfby, "") != 0) {
			tags["gsfby"] = psfby;
		}

		char gsf_path[PATH_MAX];
		sprintf(gsf_path, "%s-%04d.minigsf", gsf_basename, num);

		if (make_minigsf(gsf_path, offset, (uint32_t)size, num, tags)) {
			printf("Created %s\n", gsf_path);
		}
		else {
			printf("Error: Unable to create %s\n", gsf_path);
			num_error++;
		}
	}

	return (num_error == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
