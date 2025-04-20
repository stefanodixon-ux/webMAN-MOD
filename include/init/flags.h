#include "../../flags.h"

#ifdef REX_ONLY
 #ifndef DEX_SUPPORT
 #define DEX_SUPPORT
 #endif
#endif

#ifdef PKG_LAUNCHER
 #define MOUNT_ROMS
 #define MOUNT_GAMEI
#endif

#define FIX_CLOCK
#define OVERCLOCKING
#define VIEW_PARAM_SFO
#define CHECK_PSHOME

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
  #define NET_SUPPORT
  #define DEBUG_XREGISTRY
  #define FPS_OVERLAY
  #define MUTE_SND0
  #define DISABLE_SIGNIN_DIALOG
//#define PATCH_ROS // WARNING: This has NOT been fully tested: do not enable if you don't have a hardware flasher to test it
 #else
  #undef VISUALIZERS
  #undef NET_SUPPORT
  #undef DEBUG_XREGISTRY
  #undef VIEW_PARAM_SFO
 #endif
#else
 #undef WM_PROXY_SPRX
 #undef PS3MAPI
 #undef MOUNT_ROMS
 #undef MOUNT_GAMEI
 #undef PKG_LAUNCHER
 #undef PS3NET_SERVER
 #undef PHOTO_GUI
 #undef VISUALIZERS
 #undef NET_SUPPORT
 #undef LOAD_PRX
 #undef BDVD_REGION
#endif

#ifndef PS3MAPI
 #undef PATCH_GAMEBOOT
 #undef PS3MON_SPRX
#endif
#ifndef PATCH_GAMEBOOT
 #define set_mount_type(a)
 #define patch_gameboot(a)
 #define patch_gameboot_by_type(a)
#endif

#ifndef PS3_BROWSER
 #undef DEBUG_XREGISTRY
#endif

#ifdef LAST_FIRMWARE_ONLY
 #undef FIX_GAME
#endif

#ifndef WM_REQUEST
 #undef WM_CUSTOM_COMBO
 #undef PHOTO_GUI
#endif

#ifndef LAUNCHPAD
 #undef PHOTO_GUI
#endif

#ifndef VIRTUAL_PAD
 #undef PLAY_MUSIC
#endif

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
  #define COBRA_NON_LITE
 #endif
#endif
