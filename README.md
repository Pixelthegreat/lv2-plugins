# My LV2 Plugins
These are some of my LV2 audio plugins and effects. The recommended programs to run them in are [Reaper](https://reaper.fm) and [Muse](https://muse-sequencer.github.io), as I regularly test my plugins there.

| Name       | Short Description           | Full Description                                                                                              |
|------------|-----------------------------|---------------------------------------------------------------------------------------------------------------|
| bitcrusher | Bitcrusher                  | A bitcrusher that simulates a low quality resampling algorithm and bit rate quantization.                     |
| eq4bp      | 4-Band Parametric Equalizer | A parametric equalizer with 4 bands: One low-shelf filter, two bell / peak filters and one high-shelf filter. |

# Building and Installing
Each plugin has its own subdirectory with the code and plugin descriptions. A Makefile is provided at the root to build all of them, though each project has its own indepedent Makefile if you need to build them separately.

To install them, either copy their subdirectories to an appropriate location for LV2s (i.e. `~/.lv2`, `/usr/share/lv2`) or add the path of the cloned repository to the plugin search paths for whatever LV2 host you use.
