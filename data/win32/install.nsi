; Copyright (c) 2010 Bertrand Janin <tamentis@neopulsar.org>
;
; This is a win32 installation source file (NSIS).
;


Name "rezerwar"

; Include ModernUI2 components.
!include "MUI2.nsh"

; The file to write
OutFile install.exe


; The default installation directory
InstallDir $PROGRAMFILES\rezerwar


; Include Multi-User prompts.
!define MULTIUSER_EXECUTIONLEVEL Admin
!define MULTIUSER_MUI
!include MultiUser.nsh

Function .onInit
  !insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
FunctionEnd

; ModernUI2 stuff
!define MUI_HEADERIMAGE
!define MUI_ABORTWARNING
!define MUI_LICENSEPAGE_TEXT_TOP "Thank you for installing rezerwar, feel free to drop an email if you have any suggestions or need help."
!define MUI_LICENSEPAGE_TEXT_BOTTOM ""
!define MUI_LICENSEPAGE_BUTTON "Continue"
!define MUI_PAGE_HEADER_TEXT " "
!define MUI_PAGE_HEADER_SUBTEXT " "
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Launch rezerwar now!"
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
;!insertmacro MUI_PAGE_LICENSE "CHANGELOG"
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Function LaunchLink
  ExecShell "" "$SMPROGRAMS\rezerwar\rezerwar.lnk"
FunctionEnd

; Main installation section
Section "rezerwar, the game"

  SectionIn RO

  CreateDirectory "$INSTDIR\data"
  CreateDirectory "$INSTDIR\data\gfx"
  CreateDirectory "$INSTDIR\data\sfx"
  CreateDirectory "$INSTDIR\data\levels"
  CreateDirectory "$INSTDIR\data\music"

  SetOutPath $INSTDIR
  File src\rezerwar.exe dlls\*.dll
  SetOutPath $INSTDIR\data\gfx
  File data\gfx\*.bmp
  SetOutPath $INSTDIR\data\sfx
  File data\sfx\*.wav
  SetOutPath $INSTDIR\data\music
  File data\music\*.ogg
  SetOutPath $INSTDIR\data\levels
  File data\levels\*.lvl
  SetOutPath $INSTDIR
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\tamentis.com\rezerwar "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\rezerwar" "DisplayName" "rezerwar"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\rezerwar" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\rezerwar" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\rezerwar" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ; Delete all the Start-> stuff and re-create it.
  CreateDirectory "$SMPROGRAMS\rezerwar"
  CreateShortCut "$SMPROGRAMS\rezerwar\rezerwar.lnk" "$INSTDIR\rezerwar.exe" "" "$INSTDIR\rezerwar.exe" 0 
  CreateShortCut "$SMPROGRAMS\rezerwar\uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 
  CreateShortCut "$DESKTOP\rezerwar.lnk" "$INSTDIR\rezerwar.exe" ""

  ; Remove all the configuration stuff (to clear caching completely).
  ; RMDir /r "$LOCALAPPDATA\rezerwar"
SectionEnd


; Uninstaller
Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\rezerwar"
  DeleteRegKey HKLM "SOFTWARE\tamentis.com\rezerwar"

  ; Remove files and uninstaller
  Delete "$INSTDIR\*"
  RMDir /r "$INSTDIR"

  ; Desktop
  RMDir /r "$SMPROGRAMS\rezerwar"
  Delete "$DESKTOP\rezerwar.lnk"
SectionEnd


