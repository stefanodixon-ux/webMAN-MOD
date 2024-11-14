#define NONE -1
#define MIN(a, b)			((a) <= (b) ? (a) : (b))
#define MAX_LINE_LEN	640 // html games

extern char *stdc_73EAE03D(const char *s, int c);										// strrchr()
char* strrchr(const char *s, int c) {if(!s) return NULL; return stdc_73EAE03D(s, c);}

static int parse_lba(const char *templn, int ret_value)
{
	char *time = strrchr(templn, ' '); if(!time) return ret_value;
	char tcode[10]; // mm:ss:ff

	memcpy(tcode, time + 1, 8); tcode[8] = '\0';
	if(tcode[2]!=':' || tcode[5]!=':') return ret_value;

	unsigned int tmin, tsec, tfrm;
	tmin = (tcode[0] & 0x0F)*10 + (tcode[1] & 0x0F); // 0-98
	tsec = (tcode[3] & 0x0F)*10 + (tcode[4] & 0x0F); // 0-59
	tfrm = (tcode[6] & 0x0F)*10 + (tcode[7] & 0x0F); // 0-74

	return ((((tmin * 60) + tsec) * 75) + tfrm); // msf_to_lba
}

static int get_line(char *templn, const char *cue_buf, const int buf_size, const int start)
{
	int l = 0;
	int len = MIN(buf_size, MAX_LINE_LEN);

	int lp = start;
	bool eol = false;

	for(; lp < buf_size; lp++)
	{
		if(!cue_buf[lp]) break;

		if(cue_buf[lp] != '\n' && cue_buf[lp] != '\r')
		{
			if(eol) break;
			if(l < len) templn[l++] = cue_buf[lp];
		}
		else
			eol = true;
	}

	templn[l] = '\0';

	if(l) return lp;

	return NONE;
}

static unsigned int parse_cue(char *templn, const char *cue_buf, const int cue_size, TrackDef *tracks)
{
	if(!cue_buf || !templn || !tracks) return 1; // default 1 track

	unsigned int num_tracks = 0;

	tracks[0].lba = 0;
	tracks[0].is_audio = 0;

	if(cue_size > 16)
	{
		int lba, lp = 0, off = 0, len;

		int pregap = 0;

		while(lp < cue_size)
		{
			lp = get_line(templn, cue_buf, cue_size, lp);

			if(lp < 1) break;
			if(*templn == NULL) continue;

			lba = NONE;

			//if(strstr(templn, "INDEX 1=")) lba = get_valuen32(templn, "INDEX 1="); else // ccd frames
			if(strstr(templn, "INDEX 01") || strstr(templn, "INDEX 1 "))
			{
				len = lba = parse_lba(templn, -1); // cue msf
				if((lba < off) || !num_tracks)
					lba = off + pregap; // INDEX 01 is a track length, instead of lba in msf
				else
					len = 0;			// INDEX 01 is absolute LBA in MSF
				off = lba + len;
			}

			if(lba < 0) continue;

			if(num_tracks)
			{
				tracks[num_tracks].lba = lba;
				tracks[num_tracks].is_audio = 1;
			}

			pregap = 150; // (2 * 75) -> 2 secs * 75 frames/sec

			num_tracks++; if(num_tracks >= MAX_TRACKS) break;
		}
	}

	if(!num_tracks) num_tracks++;

	return num_tracks;
}

static int read_cue(char *path, TrackDef *tracks)
{
	char cue_buf[4096];
	u64 cue_size = 0;

	int fd;
	if(cellFsOpen(path, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		if(cellFsRead(fd, (void *)cue_buf, 4096, &cue_size) != CELL_FS_SUCCEEDED) cue_size = 0;
		cellFsClose(fd);

		return parse_cue(path, cue_buf, cue_size, tracks);
	}
	else
		return 0;
}