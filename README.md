# Echoes of the Lost

**Echoes of the Lost** (*L'Écho des Disparus*) — prototype jouable d'action-aventure 3D en C++17 / DirectX 12 pour Windows 10/11.

## Prototype : Les Larmes de la Terre

Niveau complet à Mohenjo-Daro avec :

- **Rendu DX12** — swap chain, shaders HLSL, temple inondé procédural
- **Contrôles Kaelen** — déplacement, escalade, grappin, couverture, nage
- **Énigme hydraulique** — 3 leviers pour drainer le temple
- **Combat Cartel** — patrouille IA, tir, couverture
- **Objectifs** — 4 missions avec fin de niveau

## Contrôles

| Touche | Action |
|--------|--------|
| ZQSD / WASD | Déplacement |
| Shift | Sprint |
| Espace | Saut / escalade (mur est) |
| F | Grappin (vers point jaune) |
| Q (maintenir) | Couverture |
| E | Activer levier hydraulique |
| Clic gauche | Tir |
| 1 / 2 / 3 | Pouvoirs Écho (ralenti, vision, invocation) |
| Flèches | Rotation caméra |

## Objectifs du niveau

1. Ouvrir les 3 leviers hydrauliques (E) pour drainer l'eau
2. Éliminer la patrouille du Cartel (clic gauche, utilisez les couvertures)
3. Atteindre le trône aux miroirs (zone violette, fond du temple)
4. Récupérer le premier Cristal d'Écho

## Compilation

### Prérequis

- Visual Studio 2022 (C++ Desktop)
- CMake 3.20+
- vcpkg

### Build

```powershell
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat

cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release --target EchoesOfTheLost

.\build\Release\EchoesOfTheLost.exe
```

## Structure

```
src/
├── core/         Application, JobSystem, Logger
├── game/         Echo, Player, Level, Combat, Enemies, Hydraulic puzzle
├── math/         Vec3, Mat4
├── platform/     Window, Input, Camera, Renderer (DX12)
└── networking/   Stub multijoueur (coop futur)
```

## CI/CD

GitHub Actions compile, crée l'installeur Inno Setup et publie l'artifact à chaque push sur `main`/`develop` ou tag `v*`.

## Licence

Projet en développement — tous droits réservés.
