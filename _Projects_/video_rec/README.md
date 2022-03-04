# Video Recorder plugin

This plugin is a complementary tool for webMAN MOD used to record in-game videos.

This plugin is based on the video recording feature included in Mysis' PoC plugin.


## Usage

video_rec plugin is loaded dynamically from webMAN MOD pressing `SELECT`+`R3`+`L2`+`R2`.

This is an alternative method to the internal video recorder function included in webMAN MOD.

webMAN MOD is unloaded before load video_rec plugin to reduce the memory usage during
the in-game video recording.

video_rec can be used as a standalone plugin if it is loaded directly from:
<table>
<hr><td>Path<td>Mode</hr>
<tr><td>/dev_hdd0/boot_plugins.txt<td>Cobra CEX</tr>
<tr><td>/dev_hdd0/boot_plugins_nocobra.txt<td>non-Cobra CEX</tr>
<tr><td>/dev_hdd0/boot_plugins_dex.txt<td><td>Cobra DEX</tr>
<tr><td>/dev_hdd0/boot_plugins_nocobra_dex.txt<td><td>non-Cobra DEX</tr>
</table>

Recorded videos are stored in `/dev_hdd0/VIDEO`

## Combos

- `R3` = record video using *bg memory container* (4)
- `R2` + `R3`  / `L2` + `R3` = record video using *app memory container* (1)
- `L2` + `R2` + `L3` = select default audio + video formats
- `R2` + `L3`  = change audio format
- `L2` + `L3`  = change video format
- `R3` + `L3`  = Unload video_rec.sprx

## Features

- Video formats: MPEG4, AVC MP, AVC BL, MJPEG, M4HD
- Video resolutions: 240p, 272p, 368p, 480p, 720p
- Video bitrates: 512K, 768K, 1024K, 1536K, 2048K, 5000K, 11000K, 20000K, 25000K
- Default video format: M4HD HD720 5000K 30FPS
<br>

- Audio formats: AAC, ULAW, PCM
- Audio bitrates: 96K, 128K, 64K, 384K, 768K, 1536K
- Default audio format: AAC 64K
<br>

## Limitations

In-game video recording is a non-official feature that exploits rec_plugin originally
intended for other purposes (e.g. video streaming). Therefore expect many issues like
system freeze, black screens, video not recorded, etc.

Alternatively use Mysis' PoC plugin.
[https://www.psdevwiki.com/ps3/Talk:Rec_plugin](https://www.psdevwiki.com/ps3/Talk:Rec_plugin)


## Credits

- **Mysis** - Original PoC plugin with video recording support via rec_plugin, VSH exports
- **aldostools** - Standalone video_rec plugin
