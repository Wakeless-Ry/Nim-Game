# Nim AI - Interface de Jeu

Jeu de Nim (une rangée) avec interface graphique SFML. Projet universitaire sur les algorithmes d'exploration et de mouvement, avec une IA adversaire en développement.

[![Interface Nim](app/graphs/graphe.png)](app/graphs/graphe.png)

## 1. Objectif du projet

Implémenter le jeu de Nim avec :
- Interface graphique interactive (SFML 2.5+)
- Sélection visuelle des bâtonnets
- IA adversaire (en cours de développement)
- Étude des algorithmes d'exploration

## 2. Organisation du projet

```
app/
├── bin/          # Exécutables compilés
│   └── nim_ai
├── graphs/       # Images et graphiques
│   └── graphe.png
├── includes/     # En-têtes (futurs fichiers .h)
└── src/          # Sources C++
    └── nim_game.cpp
├── CMakeLists.txt # Configuration CMake
└── README.md
```

## 3. Pré-requis

- **Git** (pour cloner le projet)
- **CMake** ≥ 3.16
- **Compilateur C++** (g++ 9+, clang++, MSVC)
- **SFML** 2.5+ (installé via gestionnaire de paquets)
- **Linux/macOS** (Windows via MSYS2/WSL)

### Installation des dépendances

**Ubuntu/Debian :**
```bash
sudo apt update
sudo apt install git cmake build-essential libsfml-dev
```

**macOS (Homebrew) :**
```bash
brew install git cmake sfml
```

**Fedora :**
```bash
sudo dnf install git cmake gcc-c++ sfml-devel
```

## 4. Récupérer le projet (branche `interface`)

```bash
# Cloner le dépôt
git clone <URL_DU_REPOSITORIUM> nim-ai
cd nim-ai

# Récupérer toutes les branches distantes
git fetch origin

# Passer sur la branche interface
git checkout interface

# Vérifier la branche active
git branch
```

> **Astuce** : Remplacez `<URL_DU_REPOSITORIUM>` par l'URL de votre dépôt Git (GitHub/GitLab).

## 🔨 Compilation

```bash
# Créer dossier build
mkdir -p build && cd build

# Configurer avec CMake
cmake ..

# Compiler (utiliser -j pour paralléliser)
make -j$(nproc)

# Exécutable généré :
../bin/nim_ai 
```

### Flags de compilation optionnels

```bash
# Release optimisé
cmake .. -DCMAKE_BUILD_TYPE=Release

# Debug avec symboles
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## 5. Exécution

```bash
# Depuis le dossier build
./bin/nim_ai
```

**Contrôles :**
- **Clic** sur les bâtonnets pour sélectionner
- **CONFIRM** pour retirer les bâtonnets sélectionnés  
- **CANCEL** pour désélectionner
- **RESET** pour recommencer

## 6. Dépannage

| Problème | Solution |
|----------|----------|
| `SFML not found` | Installer `libsfml-dev` / `sfml` |
| `Font loading failed` | Police système disponible |
| `CMake 3.16 required` | `sudo apt install cmake` |
| `Permission denied` | `chmod +x app/bin/nim_ai` |

## 7. Licence

Projet universitaire - © 2026