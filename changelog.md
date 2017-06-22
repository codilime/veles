# Veles changelog

## 2017.06.0.SWOND (2017-06-22)

### Major features and changes:

  * Added a cursor to the hex view.
  * Introduced support for editing bytes in hex view (currently without an ability to change file size).
  * Added many new keyboard shortcuts.
  * Added a shortcut editor.
  * Connection to the server uses TLS now.
  * "Replace" option has been enabled.

### Minor features and changes:

  * "Find/Replace" dialog has been slightly improved.
  * Dropped support for MinGW (due to broken std::random_device, slow compile time, lack of support for new C++ standards and many others)

### Bugfixes:

  * Client API Python library has been fixed to support Python 2.
  * Previously Ctrl+C didn't work when running srv.py on Windows.

## 2017.05.0.Świtezianka (2017-05-12)

### Major features and changes:

  * Introduced client-server architecture:
    * The server can run remotely and can be shared amongst many users.
    * The server is currently written in Python 3.
    * There's a possibility to script Veles using both Python 2 and 3, but currently without a support for any GUI operations.
  * Created an instalator for Windows, added a support for DEB packages and a drag and drop installer for macOS.
  * Dropped official 32-bit Windows support, due to problems with MessagePack.
  * Keep current state in a database file.

### Minor features and changes:

  * Added more encoders in hex view ("Copy as" context menu).
  * Added connection dialog.
  * Lowered minimum macOS version to 10.10.
  * Moved 3D view control panel to a horizontal bar.

### Bugfixes:

  * Fixed a glitch in 3D view mouse manipulation.
  * 3D views which are not currently visible don't use CPU/GPU resources anymore.
  * Fixed minor stability issues.

### Caveats:

  * Python3 (>= 3.5) is now needed to run Veles on macOS and has to be manually installed.
  * To install Veles from DEB package on Ubuntu you need to enable "universe" repository (it is normally enabled by default, but not on live CD).
  * Hotkeys from the top menu don't work on Ubuntu 16.\* due to a Qt bug:
    * This is already fixed in Qt, but 16.\* have old Qt in repo.
    * Bug description: https://bugs.launchpad.net/appmenu-qt5/+bug/1380702
    * You can circumvent the bug by running Veles with the following environment variable set: UBUNTU_MENUPROXY=""

## 2017.02.0.YAGNI (2017-02-28)

### Minor features and changes:

  * Disassembler:
    * Initial release of Python disassembler (for Python 3.x) - temporarily without specialized disassembler view.
  * Python library related improvements:
    * Added separate Python classes for blobs and chunks.
  * UI improvements:
    * HexEditWidget and NodeTreeWidget merged into single tab.
    * Various refactors and fixes.

## 2017.01.0.MechanicalTurk (2017-01-26)

### Major features and changes:

  * Added network protocol that allows using external scripts as ad-hoc parsers. Added a simple Python library to support using this functionality. Check out python/README.rst for details.
    * For security reasons network server is disabled by default and has to be enabled in the options dialog.
    * This feature is still in an early phase of development and the API will change in future releases.
  * Added a few parsers for well known file formats (avi, bmp, elf, gif, pe, mov, zip). New parsers have been generated from Kaitai template files (https://github.com/kaitai-io/kaitai_struct). Support for user-provided Kaitai files is planned in upcoming releases.
  
### Minor features and changes:

  * Major improvement in visualization performance.
  * Added camera manipulators in visualization.
  * Added reference frames and captions in visualization.
  * Added ability to parse file starting from selected location.
  * Added ability to choose which parser to use.
  * Separated hex view and tree view for file.
  * Multiple changes to docks handling.
  * Fixed multiple minor bugs in the visualization "minimap".
  * General look and feel improvements.
  * Added ability to scroll to chunk.
  * Double click in database view opens file.
  
### Bugfixes:

  * Multiple bugfixes related to visualization sampling.
  * Multiple bugfixes related to chunk deletion.
  * Fixed visualization scaling.
  * Show error message if OpenGL doesn’t work instead of crashing.
  * Added warning on failure when opening file instead of silent failure.
