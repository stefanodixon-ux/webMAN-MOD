#include <unrar.h>
#include <string.h>
#include <stdio.h>

void unrar_extract(const char* rarFilePath, const char* dstPath)
{
	HANDLE hArcData; //Archive Handle
	struct RAROpenArchiveDataEx rarOpenArchiveData;
	struct RARHeaderDataEx rarHeaderData;
	memset(&rarOpenArchiveData, 0, sizeof(rarOpenArchiveData));
	memset(&rarHeaderData, 0, sizeof(rarHeaderData));

	rarOpenArchiveData.ArcName = (char*) rarFilePath;
	rarOpenArchiveData.CmtBuf = NULL;
	rarOpenArchiveData.CmtBufSize = 0;
	rarOpenArchiveData.OpenMode = RAR_OM_EXTRACT;
	hArcData = RAROpenArchiveEx(&rarOpenArchiveData);

	printf("UnRAR Extract [%s]\n", rarFilePath);

	if (rarOpenArchiveData.OpenResult != ERAR_SUCCESS)
	{
		printf("OpenArchive '%s' Failed!\n", rarOpenArchiveData.ArcName);
		return;
	}

	while (RARReadHeaderEx(hArcData, &rarHeaderData) == ERAR_SUCCESS)
	{
		printf("Extracting '%s' (%ld) ...\n", rarHeaderData.FileName, rarHeaderData.UnpSize + (((uint64_t)rarHeaderData.UnpSizeHigh) << 32));

		if (RARProcessFile(hArcData, RAR_EXTRACT, (char*) dstPath, NULL) != ERAR_SUCCESS)
		{
			printf("ERROR: UnRAR Extract Failed!\n");
			break;
		}
	}

	RARCloseArchive(hArcData);
}

void unrar_list(const char* rarFilePath)
{
	HANDLE hArcData; //Archive Handle
	struct RAROpenArchiveDataEx rarOpenArchiveData;
	struct RARHeaderDataEx rarHeaderData;
	memset(&rarOpenArchiveData, 0, sizeof(rarOpenArchiveData));
	memset(&rarHeaderData, 0, sizeof(rarHeaderData));

	rarOpenArchiveData.ArcName = (char*) rarFilePath;
	rarOpenArchiveData.CmtBuf = NULL;
	rarOpenArchiveData.CmtBufSize = 0;
	rarOpenArchiveData.OpenMode = RAR_OM_LIST;
	hArcData = RAROpenArchiveEx(&rarOpenArchiveData);

	printf("UnRAR List [%s]\n", rarFilePath);

	if (rarOpenArchiveData.OpenResult != ERAR_SUCCESS)
	{
		printf("OpenArchive '%s' Failed!\n", rarOpenArchiveData.ArcName);
		return;
	}

	while (RARReadHeaderEx(hArcData, &rarHeaderData) == ERAR_SUCCESS)
	{
		printf("File: '%s' Size: (%ld)\n", rarHeaderData.FileName, rarHeaderData.UnpSize + (((uint64_t)rarHeaderData.UnpSizeHigh) << 32));

		if (RARProcessFile(hArcData, RAR_SKIP, NULL, NULL) != ERAR_SUCCESS)
		{
			printf("ERROR: UnRAR List Failed!");
			break;
		}
	}

	RARCloseArchive(hArcData);
}

void unrar_test(const char* rarFilePath)
{
	HANDLE hArcData; //Archive Handle
	struct RAROpenArchiveDataEx rarOpenArchiveData;
	struct RARHeaderDataEx rarHeaderData;
	memset(&rarOpenArchiveData, 0, sizeof(rarOpenArchiveData));
	memset(&rarHeaderData, 0, sizeof(rarHeaderData));

	rarOpenArchiveData.ArcName = (char*) rarFilePath;
	rarOpenArchiveData.CmtBuf = NULL;
	rarOpenArchiveData.CmtBufSize = 0;
	rarOpenArchiveData.OpenMode = RAR_OM_EXTRACT;
	hArcData = RAROpenArchiveEx(&rarOpenArchiveData);

	printf("UnRAR Test [%s]\n", rarFilePath);

	if (rarOpenArchiveData.OpenResult != ERAR_SUCCESS)
	{
		printf("OpenArchive '%s' Failed!\n", rarOpenArchiveData.ArcName);
		return;
	}

	while (RARReadHeaderEx(hArcData, &rarHeaderData) == ERAR_SUCCESS)
	{
		printf("Testing '%s' (%ld) ...\n", rarHeaderData.FileName, rarHeaderData.UnpSize + (((uint64_t)rarHeaderData.UnpSizeHigh) << 32));

		if (RARProcessFile(hArcData, RAR_TEST, NULL, NULL) != ERAR_SUCCESS)
		{
			printf("ERROR: UnRAR Test Failed!");
			break;
		}
	}

	RARCloseArchive(hArcData);
}


int main(int argc, char *argv[])
{
	// List RAR archive contents
	//unrar_list("/dev_hdd0/tmp/archive.rar");

	// Test RAR archive contents
	//unrar_test("/dev_hdd0/tmp/archive.rar");

	// Extract RAR archive contents to /dev_hdd0/tmp/
	//unrar_extract("/dev_hdd0/tmp/archive.rar", "/dev_hdd0/tmp/");

	if(argc < 2) return 0;

	unrar_extract((const char*)argv[1], (const char*)argv[2]);

	return 0;
}
