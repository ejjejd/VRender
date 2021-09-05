@echo off

call cmake -G "Visual Studio 16 2019" -Bbuild
call cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 vim -G Ninja & cmake -E copy ./vim/compile_commands.json... ./