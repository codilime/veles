[//]: # (Copyright 2018 CodiLime.
.
Licensed under the Apache License, Version 2.0 \(the "License"\).
You may not use this file except in compliance with the License.
You may obtain a copy of the License at:
.
http://www.apache.org/licenses/LICENSE-2.0
.
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
.
.
Thank you SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax
)

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

#### Formal requirement

You **NEED** to have a signed CLA sent to CodiLime before your code can be
merged. You can download one from [Contributor License
Agreement](https://www.codisec.com/cla). Sign it and then send a scan of it to
contact@codisec.com. You can open a pull request and have it reviewed while we
process your CLA.
 
#### Style
We are trying to stick as close as possible to [Google C++ Style Guide]. Refer to this document whenever in doubt.

#### Contributing pipeline

* Create your own branch. Name it `wip/your github nick/descriptive-branch-name`
* Develop your branch. Remember to:
  * keep your commit atomic (one feature per commit)
  * add descriptive commit messages. One-liners are fine for small, obvious changes, but bigger changes should have longer commit messages.
  * make sure email in your commits metadata is linked with your github account
  * Use `format` build target (for Linux run `make format`) relatively often to avoid huge commits "Style fix"
* When you think you're done open Github PR with a clear list of what you've done. From now on your branch is `public` and you cannot forcepush (`git push -f`) anything to it. If you really need to/want to pushforce FIRST ask reviewers for permission
* Every PR has to pass clang-tidy checks (`lint` build target).
* Submit fixes according to the review in new commits
* When your code will be accepted, please squash your fixes into previous feature commits
* Your code will be merged into master

More information about pull requests you can find [here](https://guides.github.com/activities/contributing-to-open-source/#contributing).

#### File placement
We have some rules as to where to place new files:
* All things that depend on QT should be placed in *ui* folder
* There should be as few things in *util* folder as possible. *util* is only for pieces of code that don't belong to any existing folders and are to small to have their own folder

<br><br>
Thanks for taking your time to contribute to Veles!

*Powered by*<br>
[<img src="https://www.codilime.com/wp-content/uploads/2016/03/codilime-color-logo-white-background-300-png.png" height="100">][Codilime About]

   [Google C++ Style Guide]: <https://google.github.io/styleguide/cppguide.html>
   [Codilime About]: <https://www.codilime.com/about/>
   
