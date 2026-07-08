; MathClav Windows installer.
;
; Expects a fully-staged application directory (MathClav.exe + Qt DLLs from
; windeployqt + MicroTeX's res/, i.e. exactly what
; `cmake --install <build-dir>` combined with app/CMakeLists.txt's
; windeployqt post-build step produces) passed in via /DSTAGING_DIR:
;
;   makensis /DSTAGING_DIR=C:\path\to\install mathclav.nsi
;
; Unverified: written against documented NSIS conventions but never run on
; a real Windows machine (this project was built in a Linux-only
; sandbox) -- see the plan doc's Phase 7 notes.

!ifndef STAGING_DIR
  !error "Pass the staged install directory: makensis /DSTAGING_DIR=<path> mathclav.nsi"
!endif
!ifndef APP_VERSION
  !define APP_VERSION "2.0.0"
!endif

!include "MUI2.nsh"

Name "MathClav"
OutFile "MathClav-${APP_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\MathClav"
InstallDirRegKey HKCU "Software\MathClav" "InstallDir"
RequestExecutionLevel admin

!define MUI_ICON "mathclav.ico"
!define MUI_UNICON "mathclav.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "French"

Section "MathClav" SEC_MAIN
    SectionIn RO
    SetOutPath "$INSTDIR"
    ; Everything windeployqt + `cmake --install` staged: MathClav.exe, Qt
    ; DLLs, platform/imageformats/etc. plugin subfolders, res/.
    File /r "${STAGING_DIR}\*.*"

    WriteRegStr HKCU "Software\MathClav" "InstallDir" "$INSTDIR"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateDirectory "$SMPROGRAMS\MathClav"
    CreateShortcut "$SMPROGRAMS\MathClav\MathClav.lnk" "$INSTDIR\MathClav.exe"
    CreateShortcut "$SMPROGRAMS\MathClav\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\MathClav.lnk" "$INSTDIR\MathClav.exe"

    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MathClav" \
        "DisplayName" "MathClav"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MathClav" \
        "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MathClav" \
        "DisplayIcon" "$INSTDIR\MathClav.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MathClav" \
        "DisplayVersion" "${APP_VERSION}"
SectionEnd

Section "Uninstall"
    RMDir /r "$INSTDIR"
    RMDir /r "$SMPROGRAMS\MathClav"
    Delete "$DESKTOP\MathClav.lnk"
    DeleteRegKey HKCU "Software\MathClav"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MathClav"
SectionEnd
