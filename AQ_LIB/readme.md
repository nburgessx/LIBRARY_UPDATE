# Algo Quant Library

## Introduction to Algo Quant Lib
This repository contains Algo Quant pricing analytics sourcecode used for building yield curves, instrument pricing and risk. Asset classes
supported include: Bonds, Swaps, Credit and Vanilla Derivatives.

## Author
[Nicholas Burgess](mailto:nburgessx@gmail.com)  

## First Time Git Usage
Firstly configure your name and email using command line git config --global user.name "[name]" and git config --global user.email "[email address]"
Secondly create a feature branch, see BitBucket Actions->Create Branch and use your name for the long-term branch names or use the new feature name
for short-term branch names. Thirdly to set-up your first workspace please clone the git repository to a folder of your choice using the git command
"git clone <URL>", note the clone URL must be copied from BitBucket under Actions->Clone and is not the general BitBucket URL. Forthly in your
workspace switch to your desired branch. You are now ready to make changes and commit.

## Working with Git Repositories
Git repositories take entire library and file snapshots. Git repositorites store file snapshots that can be compared for differences, meaning file
differences are not stored, but rather implied on demand. To checkout the library we clone the repo, we typically do this once. Once cloned we must
periodically rebase the branch to update the local branch and keep it up-to-date with the repo master. Branch updates can also be managed using the
pull command; this does a fetch and merge, which can be done separately as two steps if preferred. 

Source code changes made are committed to our local branch, which we call the staging area. Once staged we must push our branch back to the repository.
The final step is to merge our branch into the master, which is done by creating a pull request and requires approval. Pull requests are created in
Bitbucket under actions->pull request. Once approved a merge button is enabled and appears in the BitBucket pull request.

## Functional Outline  
1. Sync and Pull to update your local branch to the remote master (or rebase as necessary)
2. Commit changes to your branch
3. Push your local branch changes to your remote branch in the repository
4. Create a "Pull Request" to merge your remote branch to remote master in the repository

## Feature Branching
Simple short lived feature changes can be accommodated in with a simplified feature branching strategy
1. Create feature branch from Master.
2. Undertake all development on the feature branch and commit all changes
3. Check to see if a sync and pull or rebase is necessary
4. Create pull request for feature back to Master
5. Merge the feature branch to Master and delete the feature branch

## Git Cheat Sheet

#### INSTALLATION
  


**GitHub for Windows**  
htps://windows.github.com  
  
**Git Command Line Executable**  
Git-2.21.0-64-bit.exe  
  
**Tortoise Git**  
TortoiseGit-2.8.0.0-64bit.msi  
  
  
#### MIGRATE SVN REPOSITORY TO GIT
  
  
**$ git svn clone [URL] --no-minimize-url**  
clones a SVN repository with history without URL minimization  
i.e. without expanding from root (needed when no root access permissions)
  
  
#### CONFIGURATION
  
  
**$ git config --global user.name "[name]"**  
Sets the name you want atached to your commit transactions  

**$ git config --global user.email "[email address]"**  
Sets the email you want atached to your commit transactions    

**$ git config --global color.ui auto**  
Enables helpful colorization of command line output    
  
  
#### CREATE REPOSITORIES
  
  
**$ git init [project-name]**  
Creates a new local repository with the specified name  

**$ git clone [url]**  
Downloads a project and its entire version history  
  
  
#### REFACTOR NAMES
  
  
**$ git rm [file]**  
Deletes the file from the working directory and stages the deletion  

**$ git rm --cached [file]**  
Removes the file from version control but preserves the file locally  

**$ git mv [file-original] [file-renamed]**  
Changes the file name and prepares it for commit  
  
  
#### MAKE CHANGES
  
  
Review edits and craft a commit transaction  

**$ git status**  
Lists all new or modified files to be commited  

**$ git add [file]**  
Snapshots the file in preparation for versioning  

**$ git reset [file]**  
Unstages the file, but preserve its contents  

**$ git diff**  
Shows file differences not yet staged  

**$ git diff --staged**  
Shows file differences between staging and the last file version  

**$ git commit -m "[descriptive message]"**  
Records file snapshots permanently in version history  
  

#### UNDO CHANGES  


**$ git reset head^**  
Unstages the last commit, head^ refers to the parent of the last commit

**$ git reset [file] --hard**  
Unstages the file and deletes changes

**$ git reset [file] --soft**  
Unstages the last commit, but keeps the changes locally


#### GROUP CHANGES
  
  
Name a series of commits and combine completed efforts  

**$ git branch**  
Lists all local branches in the current repository  

**$ git branch [branch-name]**  
Creates a new branch  

**$ git checkout [branch-name]**  
Switches to the specified branch and updates the working directory  

**$ git merge [branch]**  
Combines the specified branch’s history into the current branch  

**$ git branch -d [branch-name]**  
Deletes the specified branch  
  
  
#### SUPPRESS TRACKING
  
  
Exclude temporary files and paths  
*.log  
build/  
temp-*  
A text file named .gitignore suppresses accidental versioning of files and paths matching the specified paterns  
  
**$ git ls-files --other --ignored --exclude-standard**  
Lists all ignored files in this project  
  
  
#### SAVE FRAGMENTS
  
  
Shelve and restore incomplete changes  

**$ git stash**  
Temporarily stores all modified tracked files  

**$ git stash list**  
Lists all stashed changesets  

**$ git stash pop**  
Restores the most recently stashed files  

**$ git stash drop**  
Discards the most recently stashed changeset  
  
  
#### REVIEW HISTORY
  
  
Browse and inspect the evolution of project files  

**$ git log**  
Lists version history for the current branch  

**$ git log --follow [file]**  
Lists version history for a file, including renames  

**$ git diff [first-branch]...[second-branch]**   
Shows content differences between two branches  

**$ git show [commit]**  
Outputs metadata and content changes of the specified commit  
  
  
#### REDO COMMITS
  
  
Erase mistakes and craft replacement history  

**$ git reset [commit]**  
Undoes all commits afer [commit], preserving changes locally  

**$ git reset --hard [commit]**  
Discards all history and changes back to the specified commit  
  
  
#### SYNCHRONIZE CHANGES
  
  
Register a repository bookmark and exchange version history  

**$ git fetch [bookmark]**  
Downloads all history from the repository bookmark  

**$ git merge [bookmark]/[branch]**  
Combines bookmark’s branch into current local branch  

**$ git push [alias] [branch]**  
Uploads all local branch commits to GitHub  

**$ git pull**   
Downloads bookmark history and incorporates changes  
  
  
