@echo off

set ToolDir=%~dp0

set GameName=PUBG
set PakDir=M:\pubg\20200907\TslGame\Content\Paks
set AES=45DD15D6DD2DA50AEB71CE7A5284CF8EA498B2EC3D52B7E336F3EA0071CE44B3

start %ToolDir%umodel.exe -gui -path="%PakDir%" -game="%GameName%" -aes="%AES%"