# Development notes

This page aggregates and organizes links and resources during the course of
development for the contributors' future reference.

## ibus

API documentation of ibus is available at: 

* [https://ibus.github.io/docs/ibus-1.5/index.html](https://ibus.github.io/docs/ibus-1.5/index.html)

However, most of the ibus code here is based on the following C++ adaptation.

* [ibus-libzhuyin](https://github.com/epico/ibus-libzhuyin)

The CMakeLists based install is adopted from
[libvarnam-ibus](https://github.com/varnamproject/libvarnam-ibus).


**Developing** If you are developing `ibus-slimt-t8n`, restarting `ibus-daemon`
suffices once the executable is overwritten by a more recent install.  There
are some detailed setup-instructions with screenshots through the process
available [here](https://github.com/jerinphilip/lemonade/wiki/Setting-Up-iBus). 

Logs are printed to `stderr` (slimt) and glib-log (ibus). Replacing existing
ibus-daemon by spawning a new one using `ibus-daemon -rxv` will leave a process
open and printing to the console.


**Debugging** If you want to drop to debugging, keep an `ibus-daemon` open.
With this open, we can explicitly spawn the `ibus-slimt-t8n` via a debugger
(like GDB) so it registers on D-Bus and will be picked up by the open
`ibus-daemon` instead of spawning a new one.

```bash
ibus-daemon -xrv
gdb --args /usr/local/libexec/ibus-slimt-t8n --ibus
```

If there are crashes (observable when toggling the engine on and the fallback
keyboard comes to play, it's a silent failure) - it helps pinpoint where the
issue is happening using an executable compiled with debug symbols (using
`-DCMAKE_BUILD_TYPE=Debug`).

Keep in mind to update the path with where `ibus-slimt-t8n` is installed in
your Operating System adhering to conventions.

**Useful Commands** The following commands are also useful in diagnosing
issues.

You can command line verify if ibus has loaded the engine correctly by looking
for it as follows via `ibus` command-line interface.

```bash
ibus read-cache | grep "slimt-t8n"
```

If there's output in the XML it means ibus integration is aware of
`ibus-slimt-t8n` engine.

## Launching iBus

* On the GNOME Desktop Environment, Go to **Settings > Language and Region** <br> 
  <img src="https://user-images.githubusercontent.com/727292/138954048-14f67c83-5677-4c2b-b49b-9efc68335382.png"> 
* Search for "slimt-t8n", add it among input sources. If you don't see this
  entry, try logging out of GNOME and back in again.
