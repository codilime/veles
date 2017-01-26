# Veles changelog

## 2017.01.0.MechanicalTurk (2017-01-26)

### Major features and changes:

  * Added network protocol that allows using external scripts as ad-hoc parsers. Added a simple Python library to support using this functionality. Check out python/README.rst for details.
    * For security reasons network server is disabled by default and has to be enabled in the options dialog.
    * This feature is still in an early phase of development and the API will change in future releases.
  * Added a few parsers for well known file formats (avi, bmp, elf, gif, pe, mov, zip). New parsers have been generated from Kaitai template files (https://github.com/kaitai-io/kaitai_struct). Support for user-provided Kaitai files is planned in upcoming releases.
  
### Minor features and changes:

  * Major improvement in visualisation performance.
  * Added camera manipulators in visualisation.
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

  * Multiple bugfixes related to visualisation sampling.
  * Multiple bugfixes related to chunk deletion.
  * Fixed visualisation scaling.
  * Show error message if OpenGL doesn’t work instead of crashing.
  * Added warning on failure when opening file instead of silent failure.
