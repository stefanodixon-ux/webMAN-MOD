# netiso

Proxy plugin used as intermediary between Cobra payload and ps3netsrv.

## Usage

The plugin is loaded automatically by webMAN MOD when a remote game or folder is mounted as /dev_bdvd.


# How it works

The plugins is stored in `/dev_hdd0/tmp/wm_res/netiso.sprx`.
It is loaded dynamically by webMAN MOD and unloaded from memory when the remote game is unmounted.

When a remote game is mounted, netiso plugin is loaded with the metadata needed to connect to the server.

The plugin receives the SCSCI requests sent through Cobra.
The requests are sent to ps3netsrv and data returned is sent back to Cobra payload.


## Credits

- **Cobra Team** - Original netiso concept and coding.
- **deank** - Coding and implementation in webMAN.
- **aldostools** - Modification, additional features and optimizations.
