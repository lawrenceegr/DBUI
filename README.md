# DBUI

A lightweight desktop database client built with Qt 6 and C++. Connect to databases, browse tables, run queries, and export data.

![Qt](https://img.shields.io/badge/Qt-6.x-green) ![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) ![License](https://img.shields.io/badge/license-MIT-lightgrey)

## Features

- Connect to MySQL, PostgreSQL, and SQLite databases
- Browse and inspect tables
- Execute SQL queries
- Export table data
- Save and manage connections
- Custom stylesheet theming

## Prerequisites

- CMake 3.16+
- C++17 compiler (GCC, Clang, MSVC)
- Qt 6.2+ with the following modules:
  - Qt Widgets
  - Qt Sql
  - Qt Core

### Installing dependencies

**Debian/Ubuntu:**

```bash
sudo apt install build-essential cmake qt6-base-dev
```

Install the database drivers you need:

```bash
sudo apt install libqt6sql6-sqlite   # SQLite
sudo apt install libqt6sql6-mysql    # MySQL/MariaDB
sudo apt install libqt6sql6-psql     # PostgreSQL
```

**Fedora:**

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel
```

**Arch Linux:**

```bash
sudo pacman -S cmake qt6-base
```

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The `DBUI` binary will be in the `build/` directory.

## Installing

### System install

```bash
cd build
sudo cmake --install .
```

This installs the binary to `/usr/bin/`, the desktop entry to `/usr/share/applications/`, and the icon to `/usr/share/icons/`.

### Debian package

```bash
cd build
cpack -G DEB
sudo dpkg -i dbui_0.1_amd64.deb
```

### Uninstalling

```bash
sudo dpkg -r dbui
```

## Building on Windows

Ensure Qt 6 is installed and available in your PATH or CMAKE_PREFIX_PATH, then:

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The `resources/DBUI.rc` file provides the application icon for Windows executables.
