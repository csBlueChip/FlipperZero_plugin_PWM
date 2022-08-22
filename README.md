# FlipperZero PWM Demo (Plugin)

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

You will also need to edit `applications/meta/application.fam` and add `bc_pwm_demo` to the Plugins Menu
