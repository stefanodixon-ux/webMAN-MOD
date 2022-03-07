# Boot Mamba

Boot Mamba is a package that installs Mamba payload for usage on CFW with Cobra 8.1 or later.

Use this package as alternative to Cobra payload.


## Usage

Install boot_mamba.pkg, disable Cobra and reboot.

The kernel payload is loaded through boot_plugins_kernel_nocobra.txt when Cobra is disabled.

The alternative package boot_fake_ofw.pkg starts Mamba in fake OFW mode. 
i.e. with CFW syscalls disabled.

## Files Installed

boot_mamba.pkg:

- /dev_hdd0/boot_plugins_kernel_nocobra.txt
- /dev_hdd0/boot_plugins_kernel_nocobra_dex.txt
- /dev_hdd0/boot_plugins_nocobra.txt
- /dev_hdd0/plugins/kernel/mamba_488C.bin
- /dev_hdd0/plugins/kernel/mamba_484C.bin
- /dev_hdd0/plugins/kernel/mamba_484D.bin

boot_fake_ofw.pkg:

- /dev_hdd0/boot_plugins_kernel_nocobra.txt
- /dev_hdd0/boot_plugins_nocobra.txt
- /dev_hdd0/plugins/kernel/fake_ofw_mamba_488C.bin

## Credits

- **Habib** - kernel & VSH plugin support on non-Cobra mode
- **Cobra Team** - Original Cobra payload
- **Rebug Team** - Updates and improvements to Cobra payload
- **Estwald** - Original Mamba payload
- **aldostools** - Mamba maintenance, package installer
