# Contributing to Veles
## Reporting bugs
If you found a bug in Veles please check first if the problem has been already
reported. Gather following information:
* Steps how to reproduce the problem.
* Veles version (Help/about)
* Operating system version:
  * Linux - output of `uname -a` and `cat /etc/os-release`.
  * MacOSX - output of `uname -a` and `sw_vers -productVersion`.
  * Windows - use this [guide](https://support.microsoft.com/en-us/help/13443/windows-which-operating-system).
* If running on Linux or MacOSX - output of `glxinfo | grep string`.

and create an issue on GitHub.

## Submitting changes
Please send GitHub pull request with a clear list of what you've done. Please
follow [Google C++ Style
Guide](https://google.github.io/styleguide/cppguide.html) and make sure all of
your commits are atomic (one feature per commit). Every pull request has to be
formatted using `format` build target (for Linux: run `make format`) and pass 
clang-tidy checks (`lint` target). [BUILDING.rst](BUILDING.rst) contains a
short guide describing how to configure them.

Always write a clear log message for your commits describing the change.
One-liners are fine for small, obvious changes, but bigger changes should have
longer commit messages.

More information about pull requests you can find
[here](https://guides.github.com/activities/contributing-to-open-source/#contributing).

You **NEED** to have a signed CLA sent to CodiLime before your code can be
merged. You can download one from [Contributor License
Agreement](https://www.codisec.com/cla). Sign it and then send a scan of it to
contact@codisec.com. You can open a pull request and have it reviewed while we
process your CLA.

Thanks for taking your time to contribute to Veles!
