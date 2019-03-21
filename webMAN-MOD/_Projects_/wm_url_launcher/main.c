#include <stdio.h>
#include <string.h>
#include <lv2/sysfs.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/process.h>

#define RETROARCH	"/dev_hdd0/game/SSNE10000/USRDIR/cores"
#define SHOWTIME	"/dev_hdd0/game/HTSS00003/USRDIR/movian.self"

#define VIDEO_EXTENSIONS ".MKV|.MP4|.AVI|.MPG|.MPEG|.MOV|.M2TS|.VOB|.FLV|.WMV|.ASF|.DIVX|.XVID|.PAM|.BIK|.BINK|.VP6|.MTH|.3GP|.RMVB|.OGM|.OGV|.M2T|.MTS|.TS|.TSV|.TSA|.TTS|.RM|.RV|.VP3|.VP5|.VP8|.264|.M1V|.M2V|.M4B|.M4P|.M4R|.M4V|.MP4V|.MPE|.BDMV|.DVB|.WEBM|.NSV"
#define AUDIO_EXTENSIONS ".MP3|.WAV|.WMA|.AAC|.AC3|.AT3|.OGG|.OGA|.MP2|.MPA|.M4A|.FLAC|.RA|.RAM|.AIF|.AIFF|.MOD|.S3M|.XM|.IT|.MTM|.STM|.UMX|.MO3|.NED|.669|.MP1|.M1A|.M2A|.M4B|.AA3|.OMA|.AIFC"

#include <arpa/inet.h>

#define ssend(socket, str) send(socket, str, strlen(str), 0)

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

static void urlenc(char *dst, char *src)
{
	size_t j=0;
	size_t n=strlen(src);
	for(size_t i=0; i<n; i++,j++)
	{
		     if(src[i]==' ') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '0';}
		else if(src[i] & 0x80)
		{
			dst[j++] = '%';
			dst[j++] = h2a((unsigned char)src[i]>>4);
			dst[j] = h2a(src[i] & 0xf);
		}
		else if(src[i]=='"') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '2';}
		else if(src[i]=='%') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '5';}
		else if(src[i]=='&') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '6';}
		else if(src[i]=='+') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = 'B';}
		else if(src[i]=='?') {dst[j++] = '%'; dst[j++] = '3'; dst[j] = 'F';}
		else dst[j] = src[i];
	}
	dst[j] = '\0';

	sprintf(src, "%s", dst);
}

int main(int argc, const char* argv[])
{
	char path[2048], param[2048], url[2048];

	FILE *fp;

	fp = fopen("/dev_hdd0/game/PKGLAUNCH/USRDIR/launch.txt", "rb");

	memset(path, 2048, 0);
	memset(param, 2048, 0);

	if (fp)
	{
		fread((void *) path, 1, 2048, fp);
		fclose(fp);

		if(path[0])
		{
			char *p;
			p = strstr(path, "\n"); if(p) {sprintf(param, "%s", p + 1); p[0] = 0;}
			p = strstr(path, "\r"); if(p) p[0] = 0;

			p = strstr(param, "\n"); if(p) p[0] = 0;
			p = strstr(param, "\r"); if(p) p[0] = 0;

			if(!strncmp(path, "GET ", 4))
				{urlenc(url, path); sprintf(url, "%s HTTP/1.0\r\n", path);}
			else
			if(!strncmp(path, "http://127.0.0.1/", 17) || !strncmp(path, "http://localhost/", 17))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path + 16);}
			else
			if((path[0]=='/') && (strstr(path, ".ps3") != NULL))
				{urlenc(url, path); sprintf(url, "GET %s HTTP/1.0\r\n", path);}
			else
			if((!strncmp(path, "/dev_", 5) || !strncmp(path, "/net", 4)) && ((strstr(path, "/GAME") != NULL) || (strstr(path, "/PS3ISO") != NULL) || (strstr(path, "/PSXISO") != NULL) || (strstr(path, "/PS2ISO") != NULL) || (strstr(path, "/PSPISO") != NULL) || (strstr(path, "/DVDISO") != NULL) || (strstr(path, "/BDISO") != NULL) || (strstr(path, ".ntfs[") != NULL)))
				{urlenc(url, path); sprintf(url, "GET /mount_ps3%s HTTP/1.0\r\n", path);}
		}
	}

	if(*url)
	{
		int s = connect_to_webman();
		if(s >= 0) ssend(s, url);

		fp = fopen("/dev_hdd0/game/PKGLAUNCH/USRDIR/url.txt", "wb");
		fwrite((void *) url, 1, strlen(url), fp);
		fclose(fp);

		return 0;
	}

	if(*param == 0)
	{
		if(strcasestr(path, "/ROMS/SNES/"))
		{
			struct stat s;
			sprintf(param, "%s", path); sprintf(path, "%s/snes9x2010_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/snes9x_next_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/snes9x2005_plus_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/snes9x2005_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/snes9x_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/mednafen_snes_libretro_ps3.SELF", RETROARCH);
		}
		else
		if(strcasestr(path, "/ROMS/SNES9X/"))  {sprintf(param, "%s", path); sprintf(path, "%s/snes9x_libretro_ps3.SELF", RETROARCH);}           else
		if(strcasestr(path, "/ROMS/SNES9X2005/"))  {sprintf(param, "%s", path); sprintf(path, "%s/snes9x2005_plus_libretro_ps3.SELF", RETROARCH);} else
		if(strcasestr(path, "/ROMS/SNES9X2010/"))  {sprintf(param, "%s", path); sprintf(path, "%s/snes9x2010_libretro_ps3.SELF", RETROARCH);}   else
		if(strcasestr(path, "/ROMS/SNES9X_NEXT/"))  {sprintf(param, "%s", path); sprintf(path, "%s/snes9x_next_libretro_ps3.SELF", RETROARCH);} else
		if(strcasestr(path, "/ROMS/MSNES/"))  {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_snes_libretro_ps3.SELF", RETROARCH);}     else

		if(strcasestr(path, "/ROMS/NES/"))
		{
			struct stat s;
			sprintf(param, "%s", path); sprintf(path, "%s/fceumm_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/nestopia_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/quicknes_libretro_ps3.SELF", RETROARCH);
		}
		else
		if(strcasestr(path, "/ROMS/FCEUMM/"))   {sprintf(param, "%s", path); sprintf(path, "%s/fceumm_libretro_ps3.SELF", RETROARCH);}          else
		if(strcasestr(path, "/ROMS/NESTOPIA/")) {sprintf(param, "%s", path); sprintf(path, "%s/nestopia_libretro_ps3.SELF", RETROARCH);}        else
		if(strcasestr(path, "/ROMS/QNES/"))     {sprintf(param, "%s", path); sprintf(path, "%s/quicknes_libretro_ps3.SELF", RETROARCH);}        else

		if(strcasestr(path, "/ROMS/GEN"))       {sprintf(param, "%s", path); sprintf(path, "%s/genesis_plus_gx_libretro_ps3.SELF", RETROARCH);} else
		if(strcasestr(path, "/ROMS/MEGAD"))     {sprintf(param, "%s", path); sprintf(path, "%s/genesis_plus_gx_libretro_ps3.SELF", RETROARCH);} else
		if(strcasestr(path, "/ROMS/GG"))        {sprintf(param, "%s", path); sprintf(path, "%s/gearsystem_libretro_ps3.SELF", RETROARCH);}      else

		if(strcasestr(path, "/ROMS/GBA/"))
		{
			struct stat s;
			sprintf(param, "%s", path); sprintf(path, "%s/vba_next_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/mgba_libretro_ps3.SELF", RETROARCH);
		}
		else
		if(strcasestr(path, "/ROMS/VBA/"))   {sprintf(param, "%s", path); sprintf(path, "%s/vba_next_libretro_ps3.SELF", RETROARCH);}           else
		if(strcasestr(path, "/ROMS/MGBA/"))  {sprintf(param, "%s", path); sprintf(path, "%s/mgba_libretro_ps3.SELF", RETROARCH);}               else

		if(strcasestr(path, "/ROMS/GB/"))
		{
			struct stat s;
			sprintf(param, "%s", path); sprintf(path, "%s/gambatte_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/gearboy_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/tgbdual_libretro_ps3.SELF", RETROARCH);
		}
		else
		if(strcasestr(path, "/ROMS/GBC/"))  {sprintf(param, "%s", path); sprintf(path, "%s/gambatte_libretro_ps3.SELF", RETROARCH);}            else
		if(strcasestr(path, "/ROMS/GEARBOY/"))  {sprintf(param, "%s", path); sprintf(path, "%s/gearboy_libretro_ps3.SELF", RETROARCH);}         else
		if(strcasestr(path, "/ROMS/GAMBATTE/")) {sprintf(param, "%s", path); sprintf(path, "%s/gambatte_libretro_ps3.SELF", RETROARCH);}        else
		if(strcasestr(path, "/ROMS/TGBDUAL/"))  {sprintf(param, "%s", path); sprintf(path, "%s/tgbdual_libretro_ps3.SELF", RETROARCH);}         else

		if(strcasestr(path, "/ROMS/ATARI/")){sprintf(param, "%s", path); sprintf(path, "%s/stella_libretro_ps3.SELF", RETROARCH);}              else
		if(strcasestr(path, "/ROMS/FBA/"))
		{
			struct stat s;
			sprintf(param, "%s", path); sprintf(path, "%s/fb_alpha_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/fbalpha_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/fbalpha2012_libretro_ps3.SELF", RETROARCH);
		}
		else
		if(strcasestr(path, "/ROMS/FBA2012/")) {sprintf(param, "%s", path); sprintf(path, "%s/fbalpha2012_libretro_ps3.SELF", RETROARCH);}      else
		if(strcasestr(path, "/ROMS/MAME/"))
		{
			struct stat s;
			sprintf(param, "%s", path); sprintf(path, "%s/mame078_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/mame2000_libretro_ps3.SELF", RETROARCH);
			if(stat(path, &s) != 0) sprintf(path, "%s/mame2003_libretro_ps3.SELF", RETROARCH);
		}
		else
		if(strcasestr(path, "/ROMS/MAME078/")) {sprintf(param, "%s", path); sprintf(path, "%s/mame078_libretro_ps3.SELF", RETROARCH);}          else
		if(strcasestr(path, "/ROMS/MAME2000/")) {sprintf(param, "%s", path); sprintf(path, "%s/mame2000_libretro_ps3.SELF", RETROARCH);}        else
		if(strcasestr(path, "/ROMS/MAME2003/")) {sprintf(param, "%s", path); sprintf(path, "%s/mame2003_libretro_ps3.SELF", RETROARCH);}        else

		if(strcasestr(path, "/ROMS/QUAKE/")){sprintf(param, "%s", path); sprintf(path, "%s/tyrquake_libretro_ps3.SELF", RETROARCH);}            else
		if(strcasestr(path, "/ROMS/DOOM/")) {sprintf(param, "%s", path); sprintf(path, "%s/prboom_libretro_ps3.SELF", RETROARCH);}              else

		if(strcasestr(path, "/ROMS/PCE/"))  {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_pce_fast_libretro_ps3.SELF", RETROARCH);}   else
		if(strcasestr(path, "/ROMS/PCFX/"))  {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_pcfx_libretro_ps3.SELF", RETROARCH);}      else

		if(strcasestr(path, "/ROMS/NGP/"))  {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_ngp_libretro_ps3.SELF", RETROARCH);}        else
		if(strcasestr(path, "/ROMS/VBOY/")) {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_vb_libretro_ps3.SELF", RETROARCH);}         else
		if(strcasestr(path, "/ROMS/WSWAM/")){sprintf(param, "%s", path); sprintf(path, "%s/mednafen_wswan_libretro_ps3.SELF", RETROARCH);}      else
		if(strcasestr(path, "/ROMS/SGX/"))  {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_supergrafx_libretro_ps3.SELF", RETROARCH);} else

		if(strcasestr(path, "/ROMS/ATARI2600/")){sprintf(param, "%s", path); sprintf(path, "%s/stella_libretro_ps3.SELF", RETROARCH);}          else
		if(strcasestr(path, "/ROMS/ATARI5200/")){sprintf(param, "%s", path); sprintf(path, "%s/atari800_libretro_ps3.SELF", RETROARCH);}        else
		if(strcasestr(path, "/ROMS/ATARI7800/")){sprintf(param, "%s", path); sprintf(path, "%s/prosystem_libretro_ps3.SELF", RETROARCH);}       else
		if(strcasestr(path, "/ROMS/HATARI/"))   {sprintf(param, "%s", path); sprintf(path, "%s/hatari_libretro_ps3.SELF", RETROARCH);}          else
		if(strcasestr(path, "/ROMS/LYNX/"))     {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_lynx_libretro_ps3.SELF", RETROARCH);}   else
		if(strcasestr(path, "/ROMS/JAGUAR"))    {sprintf(param, "%s", path); sprintf(path, "%s/virtualjaguar_libretro_ps3.SELF", RETROARCH);}   else

		if(strcasestr(path, "/ROMS/AMIGA/"))    {sprintf(param, "%s", path); sprintf(path, "%s/puae_libretro_ps3.SELF", RETROARCH);}            else
		if(strcasestr(path, "/ROMS/VICE/"))     {sprintf(param, "%s", path); sprintf(path, "%s/vice_x64_libretro_ps3.SELF", RETROARCH);}        else

		if(strcasestr(path, "/ROMS/FMSX/"))    {sprintf(param, "%s", path); sprintf(path, "%s/fmsx_libretro_ps3.SELF", RETROARCH);}             else
		if(strcasestr(path, "/ROMS/BMSX/"))    {sprintf(param, "%s", path); sprintf(path, "%s/bluemsx_libretro_ps3.SELF", RETROARCH);}          else

		if(strcasestr(path, "/ROMS/LUA/"))     {sprintf(param, "%s", path); sprintf(path, "%s/lutro_libretro_ps3.SELF", RETROARCH);}            else
		if(strcasestr(path, "/ROMS/ZX81/"))    {sprintf(param, "%s", path); sprintf(path, "%s/81_libretro_ps3.SELF", RETROARCH);}               else
		if(strcasestr(path, "/ROMS/FUSE/"))    {sprintf(param, "%s", path); sprintf(path, "%s/fuse_libretro_ps3.SELF", RETROARCH);}             else
		if(strcasestr(path, "/ROMS/GW/"))      {sprintf(param, "%s", path); sprintf(path, "%s/gw_libretro_ps3.SELF", RETROARCH);}               else
		if(strcasestr(path, "/ROMS/O2EM"))     {sprintf(param, "%s", path); sprintf(path, "%s/o2em_libretro_ps3.SELF", RETROARCH);}             else
		if(strcasestr(path, "/ROMS/HANDY"))    {sprintf(param, "%s", path); sprintf(path, "%s/handy_libretro_ps3.SELF", RETROARCH);}            else
		if(strcasestr(path, "/ROMS/NXENGINE")) {sprintf(param, "%s", path); sprintf(path, "%s/nxengine_libretro_ps3.SELF", RETROARCH);}         else
		if(strcasestr(path, "/ROMS/DOSBOX/"))  {sprintf(param, "%s", path); sprintf(path, "%s/dosbox_libretro_ps3.SELF", RETROARCH);}           else
		if(strcasestr(path, "/ROMS/VECX/"))    {sprintf(param, "%s", path); sprintf(path, "%s/vecx_libretro_ps3.SELF", RETROARCH);}             else
		if(strcasestr(path, "/ROMS/INTV/"))    {sprintf(param, "%s", path); sprintf(path, "%s/freeintv_libretro_ps3.SELF", RETROARCH);}         else
		if(strcasestr(path, "/ROMS/2048/"))    {sprintf(param, "%s", path); sprintf(path, "%s/2048_libretro_ps3.SELF", RETROARCH);}             else
		if(strcasestr(path, "/ROMS/POKEMINI/")){sprintf(param, "%s", path); sprintf(path, "%s/pokemini_libretro_ps3.SELF", RETROARCH);}         else
		if(strcasestr(path, "/ROMS/THEODORE/")){sprintf(param, "%s", path); sprintf(path, "%s/theodore_libretro_ps3.SELF", RETROARCH);}         else
		{
			char extension[8]; int plen = strlen(path);
			if(plen > 4)
			{
				sprintf(extension, "%s", path + plen - 4);
				if(extension[1] == '.') {extension[0] = '.', extension[1] = extension[2], extension[2] = extension[3], extension[3] = 0;}
				if(extension[2] == '.') {extension[0] = '.', extension[1] = extension[3], extension[2] = 0;}

				if(strcasestr(".SMC|.SWC|.FIG|.SFC|.ZIP|.GD3|.GD7|.DX2|.BSX", extension))
				{
					struct stat s;
					sprintf(param, "%s", path); sprintf(path, "%s/snes9x2010_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/snes9x_next_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/snes9x2005_plus_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/snes9x2005_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/snes9x_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/mednafen_snes_libretro_ps3.SELF", RETROARCH);
				}
				else
				if(strcasestr(".NES|.UNIF|.FDS", extension))
				{
					struct stat s;
					sprintf(param, "%s", path); sprintf(path, "%s/fceumm_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/nestopia_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/quicknes_libretro_ps3.SELF", RETROARCH);
				}
				else
				if(strcasestr(".MD|.MDX|.SMD|.GEN|.SMS|.GG|.SG|.BIN", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/genesis_plus_gx_libretro_ps3.SELF", RETROARCH);}   else
				if(strcasestr(".GBA", extension))
				{
					struct stat s;
					sprintf(param, "%s", path); sprintf(path, "%s/vba_next_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/mgba_libretro_ps3.SELF", RETROARCH);
				}
				else
				if(strcasestr(".GB|.GBC|.DMG", extension))
				{
					struct stat s;
					sprintf(param, "%s", path); sprintf(path, "%s/gambatte_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/gearboy_libretro_ps3.SELF", RETROARCH);
					if(stat(path, &s) != 0) sprintf(path, "%s/tgbdual_libretro_ps3.SELF", RETROARCH);
				}
				else
				if(strcasestr(".PCE", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_pce_fast_libretro_ps3.SELF", RETROARCH);}  else
				if(strcasestr(".A26", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/stella_libretro_ps3.SELF", RETROARCH);}             else
				if(strcasestr(".XFD|.ATR|.ATX|.CDM|.CAS|.A52|.XEX", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/atari800_libretro_ps3.SELF", RETROARCH);} else
				if(strcasestr(".A78", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/prosystem_libretro_ps3.SELF", RETROARCH);}          else
				if(strcasestr(".PAK", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/tyrquake_libretro_ps3.SELF", RETROARCH);}           else
				if(strcasestr(".WAD|.IWAD", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/prboom_libretro_ps3.SELF", RETROARCH);}       else
				if(strcasestr(".NGP|.NGC", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_ngp_libretro_ps3.SELF", RETROARCH);}  else
				if(strcasestr(".VB|.VBOY", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_vb_libretro_ps3.SELF", RETROARCH);}   else
				if(strcasestr(".WS|.WSC", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/mednafen_wswan_libretro_ps3.SELF", RETROARCH);} else
				if(strcasestr(".MGW", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/gw_libretro_ps3.SELF", RETROARCH);}                 else
				if(strcasestr(".J64|.JAG|.ABS|.COF", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/virtualjaguar_libretro_ps3.SELF", RETROARCH);} else
				if(strcasestr(".LNX", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/handy_libretro_ps3.SELF", RETROARCH);}              else
				if(strcasestr(".VEC", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/vecx_libretro_ps3.SELF", RETROARCH);}               else
				if(strcasestr(".EXE", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/nxengine_libretro_ps3.SELF", RETROARCH);}           else
				if(strcasestr(".ADF|.DMS|.FDI|.IPF|.UAE", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/puae_libretro_ps3.SELF", RETROARCH);}           else
				if(strcasestr(".D64|.D71|.D80|.D81|.D82|.G64||.G41|.X64|.T64|.TAP|.PRG|.P00|.CRT|.D6Z|.D7Z|.D8Z|.G6Z|.G4Z|.X6Z", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/vice_x64_libretro_ps3.SELF", RETROARCH);} else
				if(strcasestr(".P|.TZX|.T81", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/81_libretro_ps3.SELF", RETROARCH);}         else
				if(strcasestr(".LUTRO|.LUA", extension)) {sprintf(param, "%s", path); sprintf(path, "%s/lutro_libretro_ps3.SELF", RETROARCH);}       else
				if(strcasestr(VIDEO_EXTENSIONS, extension)) {sprintf(param, "%s", path); sprintf(path, "%s", SHOWTIME);} else
				if(strcasestr(AUDIO_EXTENSIONS, extension)) {sprintf(param, "%s", path); sprintf(path, "%s", SHOWTIME);}
			}
		}
	}

	if(*param)
	{
		char* launchargv[2];
		memset(launchargv, 0, sizeof(launchargv));

		launchargv[0] = (char*)malloc(strlen(param) + 1); strcpy(launchargv[0], param);
		launchargv[1] = NULL;

		sysProcessExitSpawn2((const char*)path, (char const**)launchargv, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
	}
	else
	if(*path)
	{
		sysProcessExitSpawn2((const char*)path, NULL, NULL, NULL, 0, 1001, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
	}

	return 0;
}
