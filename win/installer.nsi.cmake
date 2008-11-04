;;;
; Copyright (c) 2008, Paul Gideon Dann
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
;
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
; 3. The name of the author may not be used to endorse or promote products
;    derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
; IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
; OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
; IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
; NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
; THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;;

!include "MUI.nsh"

!define APP_SHORTNAME "@CMAKE_PROJECT_NAME@"
!define APP_LONGNAME "@APP_LONGNAME@"

!define QT_DLL_DIR "@QT_LIBRARY_DIR@\..\bin"
!define QT_PLUGINS_DIR "@QT_PLUGINS_DIR@"

Name "${APP_LONGNAME}"
OutFile "${APP_SHORTNAME}-setup.exe"
InstallDir "$PROGRAMFILES\${APP_LONGNAME}"
InstallDirRegKey HKLM "Software\${APP_LONGNAME}" ""
SetCompressor /SOLID lzma

; Request application privileges for Windows Vista
RequestExecutionLevel user

; Variables
Var StartMenuFolder

; Interface Settings
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "Licence.txt"
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${APP_LONGNAME}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APP_LONGNAME}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_SHORTNAME}.exe"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

Section Install
  SetOutPath "$INSTDIR"

  File "Licence.txt"

  ; App files
  File "${APP_SHORTNAME}.exe"

  File "${QT_DLL_DIR}\mingwm10.dll"

  File "qt.conf"
  File "${QT_DLL_DIR}\QtCore4.dll"
  File "${QT_DLL_DIR}\QtGui4.dll"
  File "${QT_DLL_DIR}\QtNetwork4.dll"

  SetOutPath $INSTDIR\plugins\imageformats
  File "${QT_PLUGINS_DIR}\imageformats\qjpeg4.dll"

  ; Uninstaller
  WriteRegStr HKLM "Software\${APP_LONGNAME}" "" $INSTDIR
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_LONGNAME}" "DisplayName" "${APP_LONGNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_LONGNAME}" "UninstallString" "$INSTDIR\Uninstall.exe"

  ; Menu folder
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APP_LONGNAME}.lnk" "$INSTDIR\${APP_SHORTNAME}.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ; Startup shortcut
  CreateShortCut "$SMSTARTUP\${APP_LONGNAME}.lnk" \
                 "$INSTDIR\${APP_SHORTNAME}.exe"
SectionEnd

Section Uninstall
	MessageBox MB_OK "Make sure ${APP_LONGNAME} is not running and press OK to continue."

  Delete "$INSTDIR\Licence.txt"

  ; App files
  Delete "$INSTDIR\${APP_SHORTNAME}.exe"

  Delete "$INSTDIR\mingwm10.dll"

  Delete "$INSTDIR\qt.conf"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"

  RMDir /r "$INSTDIR\plugins"

  ; Menu folder
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\${APP_LONGNAME}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  ; Startup shortcut
  Delete "$SMSTARTUP\${APP_LONGNAME}.lnk"

  ; Uninstaller
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey /ifempty HKLM "Software\${APP_LONGNAME}"
  DeleteRegKey /ifempty HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_LONGNAME}"
SectionEnd
