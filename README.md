# DDE Launcher

DDE Launcher is the launcher of Deepin Desktop Environment.

## Dependencies
You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies
You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.
```bash
sudo apt build-dep dde-launcher
```

2. Build
```bash
mkdir build
cd build
cmake ..
make
```

3. Install
```bash
sudo make install
```
## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/dde-launcher) 

## Getting involved

We encourage you to report issues and contribute changes

- [**Contribution guide for developers**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License
dde-launcher is licensed under [GPL-3.0-or-later](LICENSE).