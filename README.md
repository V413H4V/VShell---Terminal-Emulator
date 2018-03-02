# VShell - Terminal Emulator
Its a terminal emulator or a shell program, just like bourne shell (bash) or c-shell, that accepts commands and executes them.

##### compile vshell.c file on a unix machine as:
gcc -o vsh vshell.c
##### Execute with: ./vsh
==============================================================================================
#### Features:
##
- Enter any unix-terminal supported command to see its output.
- This shell supports upto 10 command-line parameters.
- command **showpids** shows the latest 10 child process PIDs spawned by the shell.
- command **history** shows the last 15 commands entered by the user.
- Typing **!n**, where n is the tag-number of a command in the history list, will execute that particular command.
- Pressing Ctrl+c will terminate the currently running process/command.
- Ctrl+z will suspend the curently running process.
- command **bg** will resume the currently suspended process in the background.
- command **quit** or **exit** will exit the shell.
(Underline)
#
#
#
===============================================================================================

###### Possible upgrades:
- ###### Free older commands or pids from the lists or limit the list array to use previous command's space to add-in a new commands.
- ###### Once process is suspended catch PID from SIGSTOP signal and use that PID for bg.
