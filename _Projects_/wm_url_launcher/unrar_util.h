#include <un7zip.h>
#include <unrar.h>

static void log_file(const char *path, const char *text);

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

	if (rarOpenArchiveData.OpenResult != ERAR_SUCCESS)
	{
		return;
	}

	while (RARReadHeaderEx(hArcData, &rarHeaderData) == ERAR_SUCCESS)
	{
		//printf("Extracting '%s' (%ld) ...\n", rarHeaderData.FileName, rarHeaderData.UnpSize + (((uint64_t)rarHeaderData.UnpSizeHigh) << 32));

		if (RARProcessFile(hArcData, RAR_EXTRACT, (char*) dstPath, NULL) != ERAR_SUCCESS)
		{
			break;
		}
	}

	RARCloseArchive(hArcData);
}
