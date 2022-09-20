# WARNING

This application is *__currently__* NOT working as a FAP, because the FZ kernel fails to export functions which
* were available to plugins
* are required to configure PWM

An issue has been raised: https://github.com/flipperdevices/flipperzero-firmware/issues/1747 ...It has been acknowledged on Discord, and expected to be resolved before FAP v1.0 is officially released.

WORKAROUND:
* edit `firmware/targets/f7/api_symbols.csv` and change:
  * for `calloc`
    * `Function,-,calloc,void*,"size_t, size_t"` to 
    * `Function,+,calloc,void*,"size_t, size_t"`
  * for `LL_TIM_DeInit`
    * `Function,-,LL_TIM_DeInit,ErrorStatus,TIM_TypeDef*` to 
    * `Function,+,LL_TIM_DeInit,ErrorStatus,TIM_TypeDef*`
* Update the firmware on the Flipper
  * `cd /path/to/flipper/` 
  * `./fbt flash_usb`
  
To review the code as a PLUGIN, please see this checkin: https://github.com/csBlueChip/FlipperZero_plugin_PWM/tree/16ab64c8c9a840813aadf60583acc595b79793fb

# FlipperZero PWM Demo (FAP)

This will allow you to configure a PWM timer {Frequency, Duty Cycle, Duty Scaling Curve} and attach it to a GPIO Pin.

You can currently select from:
* PB8 - Internal Speaker
* PB9 - Internal IR LED Array
* PB3 - External GPIO Pin

Internal LED, and Brightness scaling are currently not written.

```
cd ~/path/to/flipperDevKit/
mkdir -p applications_user/bc_pwm_demo
pushd applications_user/bc_pwm_demo
git clone https://github.com/csBlueChip/FlipperZero_plugin_PWM.git ./
popd
./fbt launch_app APPSRC=applications/bc_pwm_demo
```
