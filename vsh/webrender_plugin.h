#ifndef __WEBRENDER_PLUGIN_H__
#define __WEBRENDER_PLUGIN_H__
// bguerville webrender_plugin.h v0.1
typedef struct webrender_plugin_interface_t
{

		int (*PluginWakeup)(int); // 0: PluginWakeup - 1 Parameter: int value (=usually 0)
		int (*PluginWakeupWithUrl)(const char *url); // 1: PluginWakeupWithUrl - 1 Parameter: char * url
		int (*PluginWakeupWithUrlAndRestriction)(const char *url, int value); // 2: PluginWakeupWithUrlAndRestriction - 2 Parameter: char * url, int value (dis-/enable bookmark/history/setting)
		int (*Shutdown)(void); // 3: 0 Parameter:  (browser shutdown process-destroys Full Browser)
		int (*DoUnk4)(void); // 4: 1 Parameter:  motion_type
		int (*DoUnk5)(void); // 5: 
		int (*PluginWakeupWithUrlString)(int *); // 6: PluginWakeupWithUrlString
		int (*PluginWakeupWithSearchString)(int *); // 7: PluginWakeupWithSearchString
		int (*PluginWakeupWithUrlAndExitHandler)(const char *url, void *callback); // 8: PluginWakeupWithUrlAndExitHandler
		int (*DoUnk9)(void); // 9:
		int (*DoUnk10)(void); // 10:
		int (*DoUnk11)(void); // 11: SetDownloadStartCallbackWithHeader
		int (*DoUnk12)(void); // 12: SetDownloadStartCallback
		int (*DoUnk13)(void); // 13: SetDownloadCallbacks
		int (*DoUnk14)(void); // 14: DownloadStart
		int (*DoUnk15)(void); // 15: DownloadStop

} webrender_plugin_interface; // Interface Id 1

webrender_plugin_interface * webrender_interface = NULL;

typedef struct
{
	int (*Action)(const char *); // PluginWakeupWithUrl

} webrender_act0_interface; // Interface ACT0

webrender_act0_interface * webrender_act0;

typedef struct
{
	int (*DoUnk0)(void);
	int (*DoUnk1)(void);
	int (*DoUnk2)(void);
	int (*DoUnk3)(void);
	int (*DoUnk4)(void);

} webrender_plugin_mod0;

webrender_plugin_mod0 * webrender_mod0_interface;

#endif // __WEBRENDER_PLUGIN_H__
