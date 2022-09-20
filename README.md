# WARNING

This application is *__currently__* NOT working as a FAP, because the FZ kernel fails to expose functions which
* were available to plugins
* are required to configure PWM

To review the code as a PLUGIN, please see this checkin: https://github.com/csBlueChip/FlipperZero_plugin_PWM/tree/16ab64c8c9a840813aadf60583acc595b79793fb

# FlipperZero PWM Demo (FAP)

This will allow you to configure a PWM timer {Frequency, Duty Cycle, Duty Scaling Curve} and attach it to a GPIO Pin.

You can currently select from:
* PB8 - Internal Speaker
* PB9 - Internal IR LED Array
* PB3 - External GPIO Pin

Internal LED, and Brightness scaling are currently not written.

```
mkdir -p applications/bc_pwm_demo
cd applications/bc_pwm_demo
git clone https://github.com/csBlueChip/FlipperZero_plugin_PWM.git ./
```
