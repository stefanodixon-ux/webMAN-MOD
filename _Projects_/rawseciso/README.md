# raw_iso

Proxy plugin used as intermediary between Cobra payload and external storage device.


## Usage

The plugin is loaded automatically by webMAN MOD when a cached .ntfs[] file is mounted as /dev_bdvd.


# How it works

The plugins is stored in `/dev_hdd0/tmp/wm_res/raw_iso.sprx`.
It is loaded dynamically by webMAN MOD and unloaded from memory when the game is unmounted.

When a cached .ntfs[] file is mounted, raw_iso plugin is loaded with the sectors information
for the ISO obtained from the .ntfs[].

The plugin receives the SCSCI requests sent through Cobra. The physical sectors in the external
storage device are mapped to the corresponding sectors in the ISO, the sectors are read from
the storage device in raw mode and the data is returned back to Cobra payload.

The plugin also supports special modes added by Estwald to handle fake ISO and multiple PSX disc support.


## Credits

- **Cobra Team** - Original rawseciso concept and coding.
- **deank** - Coding and implementation in webMAN.
- **Estwald** - Modification, additional features and optimizations.
- **aldostools** - Modification, additional features and optimizations.
