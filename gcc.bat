@echo off
gcc -o "C:\Windows\Temp\main" "main.c" -lm -lwinmm
cmd /c start cmd /k "C:\Windows\Temp\main"