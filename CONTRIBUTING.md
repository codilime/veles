# Contributing to Veles
## Reporting bugs
If you found a bug in Veles please check first if the problem has been already
reported. Gather following information:
* Steps how to reproduce the problem.
* Veles version (Help/about)
* Operating system version:
  * Linux - output of `uname -a` and `cat /etc/os-release`.
  * MacOSX - output of `uname -a` and `sw_vers -productVersion`.
  * Windows - use this [guide][ms_guide].
* If running on Linux or MacOSX - output of `glxinfo | grep string`.

and create an issue on GitHub.

## Submitting changes

### Formal requirement

You **NEED** to have a signed CLA sent to CodiLime before your code can be
merged. You can download one from [Contributor License
Agreement](https://www.codisec.com/cla). Sign it and then send a scan of it to
contact@codisec.com. You can open a pull request and have it reviewed while we
process your CLA.
 
### Style
We are trying to stick as close as possible to [Google C++ Style Guide].
Refer to this document whenever in doubt.
Usually:
* We use `#pragma once`.
* We have to use raw pointers in many places, because of Qt.
* We don't like the Hungarian notation used for constants, so maybe we will
switch to CAPS_NOTATION at some point.

Notice you might find some older code that doesn't conform to GCS.

### Contribution pipeline

* Create your own branch. Name it `wip/your GitHub nick/descriptive-branch-name`.
* Develop your branch. Remember to:
  * Keep your commits atomic (one feature per commit).
  * Add descriptive commit messages. One-liners are fine for small,
    obvious changes, but bigger changes should have longer commit messages.
  * Make sure email in your commits metadata is linked with your GitHub
    account.
  * Use `format` build target (for Linux run `make format`) relatively
    often to avoid huge "Style fix" commits.
* When you think you're done open a GitHub PR with a clear list of what you
did. From now on your branch is *public* and you shouldn't forcepush
(`git push -f`) anything to it. If you really need to/want to pushforce
*first* ask reviewers for permission.
* Every PR has to pass clang-tidy checks (`lint` build target).
* Submit fixes according to the review in new commits.
* Once your code is accepted by reviewers, please squash your fixes
 into previous feature commits.
* Your code will be merged into master.

### File placement
We have some rules as to where to place new files:
* All things that depend on Qt should be placed in `ui` folder
* There should be as few things in `util` folder as possible. `util` is only
for pieces of code that don't belong to any existing folders and are too small
to have their own folder

<br><br>
Thanks for taking your time to contribute to Veles!

[Google C++ Style Guide]: <https://google.github.io/styleguide/cppguide.html>
[ms_guide]: <https://support.microsoft.com/en-us/help/13443/windows-which-operating-system>
