# ibus-slimt-t8n

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) 

Repurposes [Intelligent Input Method
Bus](https://en.wikipedia.org/wiki/Intelligent_Input_Bus) (iBus) to intercept
text entered into a field by a user, and insert translated text in it's place
into any graphical application.  Hence, this allows you as a user to enter text
in a language you know, while the field gets the translated text. Useful when
interacting with websites or agents in a foreign language.

See the software in action below:

<img src="https://user-images.githubusercontent.com/727292/147887982-690f5a65-ad8c-4743-8035-56f7e4f4a6b2.gif" width=720 alt="iBus translation in action"/>

The functionality will work in any GUI application which requests
keyboard-input - browser, text-editors, mail-clients, chat-clients etc.

## Setup

Setup requires the following:


* Compiler supporting C++17 or higher.
* [slimt](https://github.com/jerinphilip/slimt), which provides a
  commodity machine (most CPUs) inference engine for the neural models 
  powering machine translation.
* ibus and GLIB 2.0 libraries (for GTK and iBus)
* yaml-cpp 

**Installing Dependencies** On ArchLinux, dependencies can be installed by:

```bash
pacman -S ibus glib2
pacman -S yaml-cpp
```

For [slimt](https://github.com/jerinphilip/slimt), follow instructions in the
repository. Package management (models) is done by _slimt_, and
`ibus-slimt-t8n` is downstream to slimt. The command-line that also manages
packages can be installed via PyPI and can operate standalone.

```bash
python3 -m pip install slimt
slimt ls
slimt download
```

**Building from source** Once things are in place, run the following steps to
build and install `ibus-slimt-t8n` and associated files required for
integration into ibus.

```
# Configure.
cmake -B build -S .

# Build
cmake --build build --target all 

# Install.
sudo cmake --build build --target install
# Install the project...
# -- Install configuration: "Release"
# -- Up-to-date: /usr/local/libexec/ibus-slimt-t8n
# -- Up-to-date: /usr/share/ibus/component/slimt-t8n.xml
# -- Up-to-date: /usr/local/share/icons/bergamot.png
```

**First run** For first time install:

1. Restart GNOME (in case the entry does not appear among available input methods).
2. Restart `ibus-daemon` (`ibus-daemon -rXv`) so the ibus parent updates to be
   aware of `slimt-t8n.xml` and is capable of spawning the ibus engine
   (`ibus-slimt-t8n`).

From time to time, you may configure `ibus-slimt-t8n` using the script supplied
([`scripts/ibus-slimt-t8n-configure.py`](./scripts/ibus-slimt-t8n-configure.py),
requires python and [`slimt`](https://pypi.org/project/slimt/) python package),
installed previously.

```bash
python3 scripts/ibus-slimt-t8n-configure.py \
    --default browsermt/en-de-tiny          \
    --verify
```

The generated configuration is located at `$HOME/.config/ibus-slimt-t8n.yml`
which can be edited by hand to add your own models, as long the YAML remains
valid.

**Related Projects**

* [bergamot-translator](https://github.com/browsermt/bergamot-translator)
* [translateLocally](https://github.com/XapaJIaMnu/translateLocally)
* [slimt](https://github.com/jerinphilip/slimt)
* [lemonade](https://github.com/jerinphilip/lemonade) (This repository is a
  rebrand after reducing dependencies and using slimt).

