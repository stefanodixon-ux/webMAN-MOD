# PS3 Monitoring project by @kostirez1

It is not expected to be used by end users yet.

Functionality in this release:
- Send [InfluxDB line protocol](https://docs.influxdata.com/influxdb/v2.4/reference/syntax/line-protocol/) messages to remote instance of [Telegraf](https://www.influxdata.com/time-series-platform/telegraf/)
- Send InfluxDB lp messages to network broadcast for easier monitoring by projects like [PS3-Rich-Presence-for-Discord](https://github.com/zorua98741/PS3-Rich-Presence-for-Discord)
- Log Syscon error messages to the disk drive (#772)
- Current configuration file format .ini will most likely be replaced by .yaml in the next release, so there won't be any sample config file at the moment.
- `ps3mon.sprx` can be loaded manually by PS3MAPI or by `boot_plugins.txt` from `/dev_hdd0/tmp/wm_res/`, or by the checkbox option `PS3Mon` on setup page
