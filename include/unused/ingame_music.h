#ifdef INGAME_MUSIC

#include <cell/pad.h>
#include <cell/rtc.h>
#include <cell/cell_fs.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_music2.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_search.h>
#include <sysutil/sysutil_rec.h>

// music util data
enum {MIN_VOL = 0, DEF_VOL = 5, MAX_VOL = 9};
static char currentTrack[CELL_SEARCH_TITLE_LEN_MAX + 1];
static int16_t volumeValue[10] = { 0.0f /* off */, 0.39811f/* -8db */, 0.50119f, 0.63096f/* -4db */, 0.79433f, 1.0f/* 0 db */, 1.25893f, 1.58489f/* 4db */, 1.99526f, 2.51189f/* 8db */};

/***********************************************************************
* playback callback
***********************************************************************/
static void cb_musicutil(u32 cb_event, void *cb_param, void *cb_userData)
{
	if (cb_event == CELL_MUSIC2_EVENT_SELECT_CONTENTS_RESULT)
	{
		if( (u32)cb_param == CELL_MUSIC2_OK )
		{
			// play track immediately after selection
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_PLAY, NULL);
			getTrackInfo();
		}
	}
	else if (cb_event == CELL_MUSIC2_EVENT_SET_PLAYBACK_COMMAND_RESULT)
	{
		if ((s32)cb_param == CELL_MUSIC2_OK) getTrackInfo();
	}
}

/***********************************************************************
* track search callback
***********************************************************************/
static void cb_searchutil(CellSearchEvent cb_event, s32 cb_result, const void *cb_param, void *cb_userdata)
{
	// do nothing
}

/***********************************************************************
* load system modules
***********************************************************************/
static s32 load_modules(void)
{
	s32 ret;

	// modules for in game music
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_SEARCH);
	if (ret != CELL_OK ) return(ret);

	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_MUSIC2);
	if (ret != CELL_OK ) return(ret);

	// initialize search and music modules
	ret = cellSearchInitialize(CELL_SEARCH_MODE_NORMAL, SYS_MEMORY_CONTAINER_ID_INVALID, cb_searchutil, NULL);
	if (ret != CELL_MUSIC2_OK) return(ret);

	ret = cellMusicInitialize2(CELL_MUSIC2_PLAYER_MODE_NORMAL, 250, cb_musicutil, NULL);
	if (ret != CELL_MUSIC2_OK) return(ret);

	return CELL_OK;
}

/***********************************************************************
* unload system modules
***********************************************************************/
static s32 unload_modules(void)
{
	s32 ret;

	ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_MUSIC2);
	if (ret != CELL_OK ) return(ret);

	ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_SEARCH);
	if (ret != CELL_OK ) return(ret);

	return CELL_OK;
}

/***********************************************************************
* get track title
***********************************************************************/
static s32 getTrackInfo(void)
{
	CellSearchContentId contents_id;
	CellSearchContentType content_type;

	cellMusicGetContentsId2(&contents_id);
	u8 *infoBuffer = (u8 *)malloc(CELL_SEARCH_CONTENT_BUFFER_SIZE_MAX);
	if (!infoBuffer) return FAILED;

	cellSearchGetContentInfoByContentId(&contents_id, infoBuffer, &content_type);
	CellSearchMusicInfo *musicInfo = (CellSearchMusicInfo *)infoBuffer;
	strcpy(currentTrack, musicInfo->title);

	free(infoBuffer);

	return CELL_OK;
}

static void music_action(u8 action)
{
	switch(action)
	{
		case 0:
			load_modules();
		break;
		case 1:
			cellMusicSelectContents2();
		break;
		case 2:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_PLAY, NULL);
		break;
		case 3:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_PAUSE, NULL);
		break;
		case 4:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_STOP, NULL);
		break;
		case 5:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_NEXT, NULL);
		break;
		case 6:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_PREV, NULL);
		break;
		case 7:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_FASTFORWARD, NULL);
		break;
		case 8:
			cellMusicSetPlaybackCommand2(CELL_MUSIC2_PB_CMD_FASTREVERSE, NULL);
		break;
		case 9:
			unload_modules();
		break;
		default: // 10 - 19
			if(action < 20) cellMusicSetVolume2(volumeValue[action - 10]);
		break;
	}
}

#endif