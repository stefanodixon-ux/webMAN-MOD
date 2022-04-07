#include <stdio.h>
#include <string.h>
#include <lv2/sysfs.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/process.h>
#include <untar.h>
#include <un7zip.h>
#include <unrar.h>

#include "gui.h"

#define RETROARCH0	"/dev_hdd0//game/RETROARCH/USRDIR/cores"
#define RETROARCH1	"/dev_hdd0//game/SSNE10000/USRDIR/cores"
#define RETROARCH2	"/dev_hdd0//game/SSNE10001/USRDIR/cores"

#define SHOWTIME	"/dev_hdd0//game/HTSS00003/USRDIR/movian.self"

#define VIDEO_EXTENSIONS ".MKV|.MP4|.AVI|.MPG|.MPEG|.MOV|.M2TS|.VOB|.FLV|.WMV|.ASF|.DIVX|.XVID|.PAM|.BIK|.BINK|.VP6|.MTH|.3GP|.RMVB|.OGM|.OGV|.M2T|.MTS|.TS|.TSV|.TSA|.TTS|.RM|.RV|.VP3|.VP5|.VP8|.264|.M1V|.M2V|.M4B|.M4P|.M4R|.M4V|.MP4V|.MPE|.BDMV|.DVB|.WEBM|.NSV"
#define AUDIO_EXTENSIONS ".MP3|.WAV|.WMA|.AAC|.AC3|.AT3|.OGG|.OGA|.MP2|.MPA|.M4A|.FLAC|.RA|.RAM|.AIF|.AIFF|.MOD|.S3M|.XM|.IT|.MTM|.STM|.UMX|.MO3|.NED|.669|.MP1|.M1A|.M2A|.M4B|.AA3|.OMA|.AIFC"

#include <arpa/inet.h>

#define ssend(socket, str) send(socket, str, strlen(str), 0)

#define BUF_LEN		2048
#define MAX_LEN		2047

#include "zip_util.h"

static int sys_fs_mount(char const* deviceName, char const* deviceFileSystem, char const* devicePath, int writeProt)
{
	lv2syscall8(837, (u64) deviceName, (u64) deviceFileSystem, (u64) devicePath, 0, (u64) writeProt, 0, 0, 0 );
	return_to_user_prog(int);
}

static int connect_to_webman()
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0x7F000001;	//127.0.0.1 (localhost)
	sin.sin_port = htons(80);			//http port (80)
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
	{
		return -1;
	}

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		return -1;
	}

	return s;
}

static char h2a(char hex)
{
	char c = hex;
	if(c>=0 && c<=9)
		c += '0';
	else if(c>=10 && c<=15)
		c += 0x57; //a-f
	return c;
}

static void urldec(char *url)
{
	if(strchr(url, '%'))
	{
		u16 pos = 0; char c;
		for(u16 i = 0; url[i] >= ' '; i++, pos++)
		{
			if(url[i] == '+')
				url[pos] = ' ';
			else if(url[i] != '%')
				url[pos] = url[i];
			else
			{
				url[pos] = 0; u8 n = 2; if(url[i+1]=='u') {i++, n+=2;}
				while(n--)
				{
					url[pos] <<= 4, c = (url[++i] | 0x20);
					if(c >= '0' && c <= '9') url[pos] += c -'0';	  else
					if(c >= 'a' && c <= 'f') url[pos] += c -'a' + 10;
				}
			}
		}
		url[pos] = 0;
	}
}
static void urlenc(char *dst, char *src)
{
	size_t j = 0;
	size_t n = strlen(src);
	for(size_t i = 0; i < n; i++, j++)
	{
		if(j >= MAX_LEN) {j = MAX_LEN; break;}

			 if(src[i]==' ') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '0';}
		else if(src[i] & 0x80)
		{
			dst[j++] = '%';
			dst[j++] = h2a((unsigned char)src[i]>>4);
			dst[j]   = h2a(src[i] & 0xf);
		}
		else if(src[i] == '"') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '2';}
		else if(src[i] == '#') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '3';}
		else if(src[i] == '%') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '5';}
		else if(src[i] == '&') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '6';}
		else if(src[i] == '\''){dst[j++] = '%'; dst[j++] = '2'; dst[j] = '7';}
		else if(src[i] == '+') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = 'B';}
		else if(src[i] == '?') {dst[j++] = '%'; dst[j++] = '3'; dst[j] = 'F';}
		else dst[j] = src[i];
	}
	dst[j] = '\0';

	sprintf(src, "%s", dst);
}

static bool isdir(const char *path)
{
	struct stat s;
	if(stat(path, &s) == 0)
		return ((s.st_mode & S_IFDIR) != 0);
	else
		return 0;
}

static bool not_exists(const char *path)
{
	struct stat s;
	return (stat(path, &s) != 0);
}

static bool file_exists(const char *path)
{
	struct stat s;
	return (stat(path, &s) == 0);
}

static void log_file(const char *path, const char *text)
{
	FILE *fp = fopen(path,	"wb");
	fwrite((void *) text, 1, strlen(text), fp);
	fclose(fp);
}

int main(int argc, const char* argv[])
{
	char path[BUF_LEN], url[BUF_LEN], param[BUF_LEN];
	char *p;

	FILE *fp;

	fp = fopen("/dev_bdvd/PS3_GAME/USRDIR/launch.txt", "rb");
	if(!fp)
		fp = fopen("/dev_hdd0//game/PKGLAUNCH/USRDIR/launch.txt", "rb");

	memset(path,  BUF_LEN, 0);
	memset(param, BUF_LEN, 0);
	memset(url,   BUF_LEN, 0);

	if (fp)
	{
		fread((void *) path, 1, MAX_LEN, fp);
		fclose(fp);

		if(*path)
		{
			p = strstr(path, "\n"); if(p) {*p = 0; if(*param == 0) sprintf(param, "%s", p + 1);  p = strstr(param, "\n"); if(p) *p = 0;}
			p = strstr(path, "\r"); if(p) {*p = 0; if(*param == 0) sprintf(param, "%s", p + 1);} p = strstr(param, "\r"); if(p) *p = 0;

			if(not_exists(path))
			{
				char *retroArch = strstr(path,  "/game/SSNE10000");
				if(retroArch) memcpy(retroArch, "/game/RETROARCH", 15);
				if(not_exists(path))
				{
					char *retroArch = strstr(path,  "/game/RETROARCH");
					if(retroArch) memcpy(retroArch, "/game/SSNE10001", 15);
				}
			}
			if(!strncmp(path, "GET ", 4))
				{urlenc(url, path); sprintf(url, "%s HTTP/1.0\r\n", path);}
			else
			if(!strncmp(path, "http://127.0.0.1/", 17) || !strncmp(path, "http://localhost/", 17))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path + 16);}
			else
			if((*path == '/') && (strstr(path, ".ps3") != NULL))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path);}
			else
			if((*path == '/') && (strstr(path, "_ps3") != NULL))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path);}
			else
			if(!strncmp(path, "/mount.ps2/", 11))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path);}
			else
			if(!strncmp(path, "/mount_ps2/", 11))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path);}
			else
			if((!strncmp(path, "/dev_", 5) || !strncmp(path, "/net", 4)) && ((strstr(path, "/GAME") != NULL) || (strstr(path, "/PS3ISO") != NULL) || (strstr(path, "/PSXISO") != NULL) || (strstr(path, "/PS2ISO") != NULL) || (strstr(path, "/PSPISO") != NULL) || (strstr(path, "/DVDISO") != NULL) || (strstr(path, "/BDISO") != NULL) || (strstr(path, ".ntfs[") != NULL)))
				{urlenc(url, path); sprintf(url, "GET /mount_ps3%s HTTP/1.0\r\n", path);}
		}
	}
	else
		return 0; // launch.txt was not found

	///////////////////////
	// init screen display
	///////////////////////
	Init_Graph();

	cls();

	///////////////////////
	// process path + param
	///////////////////////
	if(strstr(path, "/EBOOT.BIN") || strstr(path, ".self") || strstr(path, ".SELF")) goto exec;

	///////////////////////
	// zip path to param
	///////////////////////
	if(*param && isdir(path) && strcasestr(param, ".zip") != NULL)
	{
		sprintf(url, "Zipping %s", path);
		DrawString(20, 216+24, url);
		SetFontAutoCenter(0);
		tiny3d_Flip();

		strcpy(url, path); if(strrchr(url, '/')) { *(strrchr(url, '/')) = 0; }
		zip_directory(url, path, param);
		return 0;
	}

	//////////////
	// extract ISO
	//////////////
	int flen = strlen(path) - 4; if(flen < 0) flen = 0;
	char *ext = path + flen; if(ext[1] == '.') ++ext;

	bool is_zip = (strcasestr(ext, ".zip") == ext);
	bool is_rar = (strcasestr(ext, ".rar") == ext);
	bool is_7z  = (strcasestr(ext, ".7z")  == ext);
	bool is_bz2 = (strcasestr(ext, ".bz2") == ext);
	bool is_tar = (strcasestr(ext, ".tar") == ext);
	bool is_tgz = (strcasestr(ext, ".tgz") == ext) || (strcasestr(ext, ".gz") == ext);

	char *dest_path = param;
	if(*dest_path != '/')
	{
		urldec(path);

		if(strstr(path, "/PSXISO")) sprintf(dest_path, "/dev_hdd0/tmp/extract/PSXISO/");
		if(strstr(path, "/PSPISO")) sprintf(dest_path, "/dev_hdd0/tmp/extract/PSPISO/");
		if(strstr(path, "/PS2ISO")) sprintf(dest_path, "/dev_hdd0/tmp/extract/PS2ISO/");
		if(strstr(path, "/PS3ISO")) sprintf(dest_path, "/dev_hdd0/tmp/extract/PS3ISO/");
		if(strstr(path, "/DVDISO")) sprintf(dest_path, "/dev_hdd0/tmp/extract/DVDISO/");
		if(strstr(path, "/BDISO"))  sprintf(dest_path, "/dev_hdd0/tmp/extract/BDISO/");

		if((is_zip || is_rar || is_bz2 || is_tgz || is_tar || is_7z) && (*dest_path == '/'))
		{
			mkdir("/dev_hdd0/tmp", 0666);
			mkdir("/dev_hdd0/tmp/extract", 0666);
			mkdirs(dest_path, 1, strlen(dest_path));

			sprintf(url, "%s%s", dest_path, "last_archive.log");
			if(file_exists(url))
			{
				fp = fopen(url, "rb");
				fread((void *) url, 1, MAX_LEN, fp);
				fclose(fp);
			}

			// extract archive
			if(strncmp(url, path, strlen(path) - 2))
			{
				// clean extract folder
				int fd; sysFSDirent dir;
				sysLv2FsOpenDir(dest_path, &fd);
				if(fd >= 0)
				{
					size_t read;
					while(!sysLv2FsReadDir(fd, &dir, &read) && read)
						if(dir.d_name[0] != '.') {sprintf(url, "%s%s", dest_path, dir.d_name); sysLv2FsUnlink(url);}
					sysLv2FsCloseDir(fd);
				}

				// Show message
				sprintf(url, "Extracting %s", path);
				DrawString(20, 216+24, url);
				SetFontAutoCenter(0);
				tiny3d_Flip();

				// log last extracted archive
				sprintf(url, "%s%s", dest_path, "last_archive.log");
				log_file(url, path);

				// extract archive
				if(is_zip)
					extract_zip(path, dest_path);
				else if(is_7z)
					Extract7zFile(path, dest_path);
				else if(is_bz2)
					untar_bz2(path, dest_path);
				else if(is_tgz)
					untar_gz(path, dest_path);
				else if(is_tar)
					untar(path, dest_path);
			}

			// find ISO
			{
				*path = 0;
				int fd; sysFSDirent dir;
				sysLv2FsOpenDir(dest_path, &fd);
				if(fd >= 0)
				{
					char *ext; size_t read;

					while(!sysLv2FsReadDir(fd, &dir, &read) && read)
					{
						ext = dir.d_name + dir.d_namlen - 4;
						if(dir.d_namlen > 4)
							if(strcasestr(".iso|.bin|.cue|.img|.mdf", ext)) {sprintf(path, "%s/%s", dest_path, dir.d_name); break;}
					}
					sysLv2FsCloseDir(fd);
				}
			}

			if(*path)
				{urlenc(url, path); sprintf(url, "GET /mount.ps3%s HTTP/1.0\r\n", path);}
			else
			{
				// clean extract folder
				int fd; sysFSDirent dir;
				sysLv2FsOpenDir(dest_path, &fd);
				if(fd >= 0)
				{
					size_t read;
					while(!sysLv2FsReadDir(fd, &dir, &read) && read)
						if(dir.d_name[0] != '.') {sprintf(url, "%s%s", dest_path, dir.d_name); sysLv2FsUnlink(url);}
					sysLv2FsCloseDir(fd);
				}

				*url = *path = 0; // do nothing & exit
			}
		}
	}

	//////////////
	// process URL
	//////////////
	if(*url)
	{
		int s = connect_to_webman();
		if(s >= 0) ssend(s, url);

		// log last URL sent to webman
		log_file("/dev_hdd0//game/PKGLAUNCH/USRDIR/url.txt", url);
		return 0;
	}

	if(not_exists(path)) return 0; // path must exists

	///////////////////////
	// extract archive
	///////////////////////
	if((is_zip && ((*param == '/') || (strstr(path, "/PS3~") != NULL))) ||
		is_rar || is_bz2 || is_tgz || is_tar || is_7z )
	{
		char *dest_path = param;

		char *filename = strrchr(path, '/');
		if(filename)
		{
			++filename;
			if(!strncmp(filename, "PS3~", 4))
			{
				int len = sprintf(dest_path, "/%s", filename + 4);
				for(int i = 1; i < len; i++) if(*(dest_path + i) == '~') *(dest_path + i) = '/';
				*(dest_path + len - (is_7z ? 3 : 4)) = 0; // remove zip

				if(!strncmp(dest_path, "/dev_blind/", 11))
					sys_fs_mount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0);
			}
			else if(*dest_path != '/')
			{
				char old = *filename; *filename = 0;
				sprintf(dest_path, "%s", path);
				*filename = old;
			}
		}

		if(*dest_path == '/')
		{
			int len = strlen(dest_path); if(len > 0) --len;
			if(dest_path[len] != '/') strcat(dest_path, "/");

			// Show message
			sprintf(url, "Extracting %s", path);
			DrawString(20, 216+24, url);
			sprintf(url, "To %s", dest_path);
			DrawString(20, 248+24, url);
			SetFontAutoCenter(0);
			tiny3d_Flip();

			if(is_zip)
				extract_zip(path, dest_path);
			else if(is_7z)
				Extract7zFile(path, dest_path);
			else if(is_bz2)
				untar_bz2(path, dest_path);
			else if(is_tgz)
				untar_gz(path, dest_path);
			else if(is_tar)
				untar(path, dest_path);
			else if(is_rar && file_exists("/dev_hdd0//game/PKGLAUNCH/USRDIR/unrar.self"))
			{
				char* launchargv[3];
				memset(launchargv, 0, sizeof(launchargv));

				p = strstr(param, "\n"); if(p) *p = 0;
				p = strstr(param, "\r"); if(p) *p = 0;

				launchargv[0] = (char*)malloc(strlen(path) + 1); strcpy(launchargv[0], path);
				launchargv[1] = (char*)malloc(strlen(dest_path) + 1); strcpy(launchargv[1], dest_path);
				launchargv[2] = NULL;

				sysProcessExitSpawn2("/dev_hdd0//game/PKGLAUNCH/USRDIR/unrar.self", (char const**)launchargv, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
			}
		}

		return 0;
	}

	// find executable
	if(*param == 0)
	{
		const char retro[3][40] = {RETROARCH0, RETROARCH1, RETROARCH2};
		const char sufix[3][24] = {"_libretro_ps3.SELF", "_libretro_ps3.SELF", "_libretro_psl1ght.SELF"};

		sprintf(param, "%s", path);

		if(strstr(path, "/dev_bdvd/") == path) sprintf(path, "/ROMS/%s", param + 10);

		char *RETROARCH;
		for(int i = 0; i < 3; i++)
		{
			RETROARCH = (char*)retro[i];

			if(not_exists(RETROARCH)) continue;

			if(strcasestr(path, "/ROMS/SNES/"))
			{
									 sprintf(path, "%s/snes9x2010%s",	  RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/snes9x_next%s",	 RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/snes9x2005_plus%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/snes9x2005%s",	  RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/snes9x%s",		  RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mednafen_snes%s",   RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/SNES9X/"))	   sprintf(path, "%s/snes9x%s",		  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/SNES9X2005/"))  sprintf(path, "%s/snes9x2005%s",	  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/SNES9X2005P"))  sprintf(path, "%s/snes9x2005_plus%s", RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/SNES9X2010/"))  sprintf(path, "%s/snes9x2010%s",	  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/SNES9X_NEXT/")) sprintf(path, "%s/snes9x_next%s",	 RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/MSNES/"))	   sprintf(path, "%s/mednafen_snes%s",   RETROARCH, sufix[i]); else

			if(strcasestr(path, "/ROMS/NES/"))
			{
									 sprintf(path, "%s/fceumm%s",   RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/nestopia%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/quicknes%s", RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/FCEUMM/"))   sprintf(path, "%s/fceumm%s",   RETROARCH, sufix[i]);		else
			if(strcasestr(path, "/ROMS/NESTOPIA/")) sprintf(path, "%s/nestopia%s", RETROARCH, sufix[i]);		else
			if(strcasestr(path, "/ROMS/QNES/"))		sprintf(path, "%s/quicknes%s", RETROARCH, sufix[i]);		else
			if(strcasestr(path, "/ROMS/QUICKNES/")) sprintf(path, "%s/quicknes%s", RETROARCH, sufix[i]);		else

			if(strcasestr(path, "/ROMS/GEN"))
			{
									 sprintf(path, "%s/genesis_plus_gx_wide%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/genesis_plus_gx%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/picodrive%s",	   RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/MEGAD"))
			{
									 sprintf(path, "%s/genesis_plus_gx_wide%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/genesis_plus_gx%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/picodrive%s",	   RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/PICO"))	 sprintf(path, "%s/picodrive%s",		RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/MASTER")) sprintf(path, "%s/genesis_plus_gx%s",	RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/SEGACD")) sprintf(path, "%s/genesis_plus_gx%s",	RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/GG"))	 sprintf(path, "%s/gearsystem%s",		RETROARCH, sufix[i]); else

			if(strcasestr(path, "/ROMS/GBA/"))
			{
									 sprintf(path, "%s/vba_next%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mgba%s",	 RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/gpsp%s",	 RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/VBA"))   sprintf(path, "%s/vba_next%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MGBA/")) sprintf(path, "%s/mgba%s",	 RETROARCH, sufix[i]);		else
			if(strcasestr(path, "/ROMS/GPSP/")) sprintf(path, "%s/gpsp%s",	 RETROARCH, sufix[i]);		else

			if(strcasestr(path, "/ROMS/GB/"))
			{
									 sprintf(path, "%s/gambatte%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/gearboy%s",  RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/tgbdual%s",  RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/GBC/"))	    sprintf(path, "%s/gambatte%s", RETROARCH, sufix[i]);	  else
			if(strcasestr(path, "/ROMS/GEARBOY/"))  sprintf(path, "%s/gearboy%s",  RETROARCH, sufix[i]);	  else
			if(strcasestr(path, "/ROMS/GAMBATTE/")) sprintf(path, "%s/gambatte%s", RETROARCH, sufix[i]);	  else
			if(strcasestr(path, "/ROMS/TGBDUAL/"))  sprintf(path, "%s/tgbdual%s",  RETROARCH, sufix[i]);	  else

			if(strcasestr(path, "/ROMS/ATARI/"))
			{
									 sprintf(path, "%s/stella2014%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/stella%s",	 RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/FBA/"))
			{
									 sprintf(path, "%s/fb_alpha%s",	RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/fbalpha%s",	 RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/fbalpha2012%s", RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/FBALPHA/")) sprintf(path, "%s/fbalpha%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/FBA2012/")) sprintf(path, "%s/fbalpha2012%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/FBNEO/"))   sprintf(path, "%s/fbneo%s",	   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/NEOCD/"))   sprintf(path, "%s/neocd%s",	   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/NEO"))
			{
									 sprintf(path, "%s/fbneo_light%s",	RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/fbneo%s",		RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/neocd%s",		RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/MAME/"))
			{
									 sprintf(path, "%s/mame2003%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mame2000%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mame2003_plus%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mame2003_midway%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mame078%s", RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/MAME078/"))  sprintf(path, "%s/mame078%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MAME2000/")) sprintf(path, "%s/mame2000%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MAME2003/")) sprintf(path, "%s/mame2003%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MAMEPLUS/")) sprintf(path, "%s/mame2003_plus%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MIDWAY/"))   sprintf(path, "%s/mame2003_midway%s", RETROARCH, sufix[i]); else

			if(strcasestr(path, "/ROMS/QUAKE/"))  sprintf(path, "%s/tyrquake%s",   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/DOOM/"))   sprintf(path, "%s/prboom%s",	 RETROARCH, sufix[i]);		else

			if(strcasestr(path, "/ROMS/PCE/"))
			{
									 sprintf(path, "%s/mednafen_pce_fast%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mednafen_pce%s",	  RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/MPCE/")) sprintf(path, "%s/mednafen_pce%s",		  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/PCFX/")) sprintf(path, "%s/mednafen_pcfx%s",		  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/PCECD/"))sprintf(path, "%s/mednafen_pce_fast%s",	  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/NGP"))   sprintf(path, "%s/mednafen_ngp%s",		  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/VBOY/")) sprintf(path, "%s/mednafen_vb%s",		  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/WSWAM")) sprintf(path, "%s/mednafen_wswan%s",	  RETROARCH, sufix[i]); else
			if(strcasestr(path, "/ROMS/SGX/"))
			{
									 sprintf(path, "%s/mednafen_supergrafx%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/mednafen_pce%s", RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/ATARI2600/"))
			{
									 sprintf(path, "%s/stella2014%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/stella%s",	  RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/ATARI800/")) sprintf(path, "%s/atari800%s",		RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/ATARI5200/"))sprintf(path, "%s/a5200%s",			RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/ATARI7800/"))sprintf(path, "%s/prosystem%s",		RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/HATARI/"))	sprintf(path, "%s/hatari%s",		RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/LYNX/"))		sprintf(path, "%s/mednafen_lynx%s",	RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/JAGUAR"))	sprintf(path, "%s/virtualjaguar%s", RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/STELLA2014"))
			{
									 sprintf(path, "%s/stella2014%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/stella%s",	  RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/STELLA"))
			{
									 sprintf(path, "%s/stella%s",	  RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/stella2014%s", RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/AMIGA/"))	sprintf(path, "%s/puae%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/CD32/"))		sprintf(path, "%s/puae%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/VICE/"))		sprintf(path, "%s/vice_x64%s",	RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/X64/"))		sprintf(path, "%s/vice_x64%s",  RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/X64SC/"))	sprintf(path, "%s/vice_x64sc%s",  RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/X64DTV/"))	sprintf(path, "%s/vice_x64dtv%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/X128/"))		sprintf(path, "%s/vice_x128%s",   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XPET/"))		sprintf(path, "%s/vice_xpet%s",   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XVIC/"))		sprintf(path, "%s/vice_xvic%s",   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XCBM2/"))	sprintf(path, "%s/vice_xcbm2%s",  RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XCBM5"))		sprintf(path, "%s/vice_xcbm5x0%s",RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XPLUS4/"))	sprintf(path, "%s/vice_xplus4%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XSCPU64/"))	sprintf(path, "%s/vice_xscpu64%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/XCBM25X0/"))	sprintf(path, "%s/vice_xcbm5x0%s", RETROARCH, sufix[i]);	else

			if(strcasestr(path, "/ROMS/COLECO/"))
			{
									 sprintf(path, "%s/gearcoleco%s",RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/bluemsx%s",	 RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/FMSX/"))
			{
									 sprintf(path, "%s/fmsx%s",		RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/bluemsx%s",	RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/BMSX/"))		sprintf(path, "%s/bluemsx%s",	RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MSX/"))		sprintf(path, "%s/bluemsx%s",	RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MSX2/"))		sprintf(path, "%s/bluemsx%s",	RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/SG1000/"))	sprintf(path, "%s/bluemsx%s",	RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/BLUEMSX/"))	sprintf(path, "%s/bluemsx%s",	RETROARCH, sufix[i]);	else

			if(strcasestr(path, "/ROMS/ZX81/"))		sprintf(path, "%s/81%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/FUSE/"))		sprintf(path, "%s/fuse%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/GW/"))		sprintf(path, "%s/gw%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/O2EM"))		sprintf(path, "%s/o2em%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/HANDY"))		sprintf(path, "%s/handy%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/NXENGINE"))	sprintf(path, "%s/nxengine%s",	RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/DOSBOX/"))
			{
									 sprintf(path, "%s/dosbox_svn%s", RETROARCH, sufix[i]);
				if(not_exists(path)) sprintf(path, "%s/dosbox%s",	  RETROARCH, sufix[i]);
			}
			else
			if(strcasestr(path, "/ROMS/DOS")) sprintf(path, "%s/dosbox%s",	 RETROARCH, sufix[i]); else

			if(strcasestr(path, "/ROMS/2048/"))		sprintf(path, "%s/2048%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/CANNONBALL/")) sprintf(path, "%s/cannonball%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/CAP32/"))	sprintf(path, "%s/cap32%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/DESMUME"))   sprintf(path, "%s/desmume2015%s",  RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/DS/"))       sprintf(path, "%s/desmume2015%s",  RETROARCH, sufix[i]);  else
			if(strcasestr(path, "/ROMS/GME/"))		sprintf(path, "%s/gme%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/JAVAME"))	sprintf(path, "%s/squirreljme%s",RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/JME"))		sprintf(path, "%s/squirreljme%s",RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/JUMP"))		sprintf(path, "%s/jumpnbump%s",  RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/INTV/"))		sprintf(path, "%s/freeintv%s",   RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MU/"))		sprintf(path, "%s/mu%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/PALM/"))		sprintf(path, "%s/mu%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/POKEMINI/")) sprintf(path, "%s/pokemini%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/SCUMMVM/"))	sprintf(path, "%s/scummvm%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/SQUIRREL"))	sprintf(path, "%s/squirreljme%s",RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/THEODORE/")) sprintf(path, "%s/theodore%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/VECX/"))		sprintf(path, "%s/vecx%s",		 RETROARCH, sufix[i]);	else

			// Cores formerly included in RetroArch but not supported on PS3
			if(strcasestr(path, "/ROMS/BK/"))		sprintf(path, "%s/bk%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/BOMBER"))	sprintf(path, "%s/mrboom%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/MRBOOM/"))	sprintf(path, "%s/mrboom%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/ECWOLF/"))	sprintf(path, "%s/ecwolf%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/FREECHAF/"))	sprintf(path, "%s/freechaf%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/LUA/"))		sprintf(path, "%s/lutro%s",		RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/NEKOP2/"))	sprintf(path, "%s/nekop2%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/NP2KAI/"))	sprintf(path, "%s/np2kai%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/QUAKE2/"))	sprintf(path, "%s/vitaquake2%s", RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/QUASI88/"))	sprintf(path, "%s/quasi88%s",	 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/TEST/"))		sprintf(path, "%s/test%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/UZEM/"))		sprintf(path, "%s/uzem%s",		 RETROARCH, sufix[i]);	else
			if(strcasestr(path, "/ROMS/X1/"))		sprintf(path, "%s/x1%s",		 RETROARCH, sufix[i]);	else
			{
				char extension[8]; int plen = strlen(path);
				if(plen > 4)
				{
					sprintf(extension, "%s", path + plen - 4);
					if(extension[1] == '.') {extension[0] = '.', extension[1] = extension[2], extension[2] = extension[3], extension[3] = 0;}
					if(extension[2] == '.') {extension[0] = '.', extension[1] = extension[3], extension[2] = extension[3] = 0;}

					if(strcasestr(".SMC|.SWC|.FIG|.SFC|.ZIP|.GD3|.GD7|.DX2|.BSX", extension))
					{
											 sprintf(path, "%s/snes9x2010%s",	  RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/snes9x_next%s",	 RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/snes9x2005_plus%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/snes9x2005%s",	  RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/snes9x%s",		  RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/mednafen_snes%s",   RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".NES|.UNIF|.FDS|.UNF", extension))
					{
											 sprintf(path, "%s/fceumm%s",   RETROARCH, sufix[i]); // Nintendo - NES / Famicom (FCEUmm)
						if(not_exists(path)) sprintf(path, "%s/nestopia%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/quicknes%s", RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".MD|.MDX|.SMD|.GEN|.SMS|.GG|.SG|.BIN|.32X|.68K", extension))
					{
											 sprintf(path, "%s/genesis_plus_gx_wide%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/genesis_plus_gx%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/picodrive%s",	   RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".GBA", extension))
					{
											 sprintf(path, "%s/vba_next%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/mgba%s",	 RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/gpsp%s",	 RETROARCH, sufix[i]);
					}
					if(strcasestr(".JAR", extension))
					{
						sprintf(path, "%s/squirreljme%s", RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".GB|.GBC|.DMG", extension))
					{
											 sprintf(path, "%s/gambatte%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/gearboy%s",  RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/tgbdual%s",  RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".DSK|.SNA|.TAP|.CDT|.VOC", extension))
					{
						sprintf(path,"%s/cap32%s", RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".PRC|.PQA|.IMG", extension))
					{
						sprintf(path,"%s/mu%s", RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".ROM|.RI|.MX1|.MX2|.COL|.SC", extension))
					{
											 sprintf(path, "%s/bluemsx%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/fmsx%s",	RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".CHD", extension))	   sprintf(path, "%s/neocd%s",			 RETROARCH, sufix[i]);  else
					if(strcasestr(".PCE", extension))
					{
											 sprintf(path, "%s/mednafen_pce_fast%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/mednafen_pce%s",	  RETROARCH, sufix[i]);
					}
					if(strcasestr(".A26", extension))
					{
											 sprintf(path, "%s/stella2014%s", RETROARCH, sufix[i]);
						if(not_exists(path)) sprintf(path, "%s/stella%s",	 RETROARCH, sufix[i]);
					}
					else
					if(strcasestr(".UZE", extension))	   sprintf(path, "%s/uzem%s",			  RETROARCH, sufix[i]);  else
					if(strcasestr(".7z", extension))		sprintf(path, "%s/fbneo%s",			 RETROARCH, sufix[i]);  else // Arcade (FinalBurn Neo)
					if(strcasestr(".XFD|.ATR|.ATX|.CDM|.CAS|.A52|.XEX", extension)) sprintf(path, "%s/atari800%s", RETROARCH, sufix[i]); else
					if(strcasestr(".A78", extension))	   sprintf(path, "%s/prosystem%s",	  RETROARCH, sufix[i]);	 else
					if(strcasestr(".PAK", extension))	   sprintf(path, "%s/tyrquake%s",	   RETROARCH, sufix[i]);	 else
					if(strcasestr(".WAD|.IWAD", extension)) sprintf(path, "%s/prboom%s",		 RETROARCH, sufix[i]);	 else
					if(strcasestr(".NGP|.NGC", extension))  sprintf(path, "%s/mednafen_ngp%s",   RETROARCH, sufix[i]);	 else
					if(strcasestr(".VB|.VBOY", extension))  sprintf(path, "%s/mednafen_vb%s",	RETROARCH, sufix[i]);	 else
					if(strcasestr(".WS|.WSC", extension))   sprintf(path, "%s/mednafen_wswan%s", RETROARCH, sufix[i]);	 else
					if(strcasestr(".MGW", extension))	   sprintf(path, "%s/gw%s",			 RETROARCH, sufix[i]);	 else
					if(strcasestr(".J64|.JAG|.ABS|.COF", extension)) sprintf(path, "%s/virtualjaguar%s", RETROARCH, sufix[i]); else
					if(strcasestr(".LNX", extension))	   sprintf(path, "%s/handy%s",	RETROARCH, sufix[i]);		   else
					if(strcasestr(".VEC", extension))	   sprintf(path, "%s/vecx%s",	 RETROARCH, sufix[i]);		   else
					if(strcasestr(".EXE", extension))	   sprintf(path, "%s/nxengine%s", RETROARCH, sufix[i]);		   else
					if(strcasestr(".ADF|.DMS|.FDI|.IPF|.UAE", extension)) sprintf(path, "%s/puae%s", RETROARCH, sufix[i]); else
					if(strcasestr(".D64|.D71|.D80|.D81|.D82|.G64||.G41|.X64|.T64|.TAP|.PRG|.P00|.CRT|.D6Z|.D7Z|.D8Z|.G6Z|.G4Z|.X6Z", extension)) sprintf(path, "%s/vice_x64%s", RETROARCH, sufix[i]); else
					if(strcasestr(".P|.TZX|.T81", extension)) sprintf(path, "%s/81%s",		RETROARCH, sufix[i]);		  else
					if(strcasestr(".LUTRO|.LUA", extension))  sprintf(path, "%s/lutro%s",	 RETROARCH, sufix[i]);		  else
					if(strcasestr(".CHF", extension))		 sprintf(path, "%s/freechaf%s",  RETROARCH, sufix[i]);		  else // Fairchild ChannelF (FreeChaF)
					if(strcasestr(VIDEO_EXTENSIONS, extension)) {sprintf(path, "%s", SHOWTIME); break;} else
					if(strcasestr(AUDIO_EXTENSIONS, extension)) {sprintf(path, "%s", SHOWTIME); break;}
				}
			}
			if(file_exists(path)) break;
		}

		if(not_exists(path)) *param = 0;
	}

exec:

	if(*param)
	{
		char* launchargv[2];
		memset(launchargv, 0, sizeof(launchargv));

		p = strstr(param, "\n"); if(p) *p = 0;
		p = strstr(param, "\r"); if(p) *p = 0;

		launchargv[0] = (char*)malloc(strlen(param) + 1); strcpy(launchargv[0], param);
		launchargv[1] = NULL;

		sysProcessExitSpawn2((const char*)path, (char const**)launchargv, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
	}
	else
		sysProcessExitSpawn2((const char*)path, NULL, NULL, NULL, 0, 1001, SYS_PROCESS_SPAWN_STACK_SIZE_1M);

	return 0;
}
