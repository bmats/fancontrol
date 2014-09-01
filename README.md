# Fan Control

A small system tray utility to control a MacBook Pro's fans from Windows in Boot Camp. Let the utility automatically control the fan speed based on hardware temperatures, or set the fan speed manually.

![Fan Control Screenshot](https://github.com/bmats/fancontrol/raw/master/images/context.png)

This project uses code from [Lubbo's MacBook Pro Fan Control](http://sourceforge.net/projects/lubbofancontrol/). It also incorporates the fix for obtaining accurate CPU temperatures on newer MacBooks from [here](http://sourceforge.net/p/lubbofancontrol/discussion/970570/thread/8b89d2ad/#df5a).

Built with [Qt](http://qt-project.org/).


## Use

### Modes

__Auto mode__ sets the fan speed based on the temperature of the CPU and GPU. The fan speed range you set corresponds to an allowed temperature range. By default, the temperature range is from 30°C to 80°C, the CPU controls the left fan, and the GPU controls the right fan, but this can be configured in the preferences window.

__Manual mode__ sets the fans to specific speeds between 1200 RPM and 6000 RPM.

### Tips

To increase the fan speed when in auto mode, either move the lower slider RPM up or increase the `Auto Temperature Range` slider's lower value in the preferences window.


### Adding custom sensors

Create a JSON file named `sensors.json` in the same directory as the executable to add sensors to display in the temperatures window or to override the built-in sensor descriptions. The JSON should be structured like this:

```json
[
  { "key": "TC0D", "name": "CPU A (diode)" },
  { "key": "TC0H", "name": "CPU A (heatsink)" },
  { "key": "TC0P", "name": "CPU A (proximity)" },
  ...
]
```

`key` is required for each sensor, and `name` is optional. If name is omitted, the key is displayed in the temperature window instead.


## Disclaimer

Use this utility at your own risk. You take full responsibility for any consequences arising from the use of this utility. This program makes no guarantees that it will keep your computer from overheating. Any damage to hardware is your own fault! See [the license](LICENSE) for more information.

Important things to know:

- Your computer can overheat, even when this program sets your computer's fans to the maximum safe speed.
- While this program can warn you if your computer is getting too hot, it is possible to turn off these notifications (or just ignore them), and your computer could overheat.
- Usually, your computer will turn itself off automatically if it gets too hot.
- If you're in the middle of work and your computer is overheating, put it to sleep. This will save your work and give your computer time to cool off.

Both this program and Lubbo's Fan Control were designed and tested only on MacBook Pros using NVIDIA GPUs.


## Licenses

Licensed under the GNU General Public License version 3.

Qt is also licensed under the GNU GPL v3. Lubbo's Fan Control is licensed under the GNU GPL v2.

See the license information for [NVAPI](https://developer.nvidia.com/nvapi) and [LibQxt](http://libqxt.org/) in their respective source files.
