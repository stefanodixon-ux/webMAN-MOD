#include <fcntl.h>
#include <ppu-lv2.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>

//#include <io/pad.h>

#define SUCCESS 0
#define FAILED -1

#define SC_COBRA_SYSCALL8								8
#define SYSCALL8_OPCODE_LOAD_VSH_PLUGIN					0x1EE7
#define SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN				0x364F

int cobra_load_vsh_plugin(unsigned int slot, const char *path, void *arg, uint32_t arg_size)
{
	lv2syscall5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, slot, (uint64_t)(uint32_t*)path, (uint64_t)(uint32_t*)arg, arg_size);
	return (int)p1;
}

int cobra_unload_vsh_plugin(unsigned int slot)
{
	lv2syscall2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN, slot);
	return (int)p1;
}

int main()
{
	cobra_unload_vsh_plugin(5);
	cobra_load_vsh_plugin(5, "/dev_hdd0/game/UTBSCRIPT/USRDIR/aiscript.sprx", NULL, 0);

	return SUCCESS;
}
