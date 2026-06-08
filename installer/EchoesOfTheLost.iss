[Setup]
AppName=Echoes of the Lost
AppVersion=0.1.0
AppPublisher=Echoes of the Lost Team
DefaultDirName={autopf}\EchoesOfTheLost
DefaultGroupName=Echoes of the Lost
UninstallDisplayIcon={app}\EchoesOfTheLost.exe
Compression=lzma2
SolidCompression=yes
OutputDir=..\Output
OutputBaseFilename=EchoesOfTheLost_Setup
SetupIconFile=..\resources\EchoesOfTheLost.ico
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Files]
Source: "..\staging\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Echoes of the Lost"; Filename: "{app}\EchoesOfTheLost.exe"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Echoes of the Lost"; Filename: "{app}\EchoesOfTheLost.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Créer un raccourci sur le bureau"; GroupDescription: "Raccourcis:"

[Run]
Filename: "{app}\EchoesOfTheLost.exe"; Description: "Lancer le jeu"; Flags: postinstall nowait skipifsilent
