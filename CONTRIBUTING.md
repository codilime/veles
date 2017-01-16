# Git Workflow
## Base branches
### master
  
  Contains mainline of code, ultimately where everything gets merged.
  A release is tagged in this branch.
  
### wip/$user/$whatever
  
  Contains any kind of work-in-progress has not been relegated to another branch
  
### bug/${issue}-title
  
  Fixes specific to a bug
  
### release/${major}.${minor}
  
  Branches for backported fixes (Loong in the future)
  
## Tags
Tags should have the form of _MAJOR_._MINOR_, optionally with an extra number for point release (in maintenance branches)
