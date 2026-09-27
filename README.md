# Nim AI - Intelligence Artificielle pour le jeu de Nim

## Auteurs du projet  
ORAVEC Tommy-Verdi  
RODRIGUES Ryan  
TEXIER--GRAILHES Killian  

## Description du projet

Ce projet universitaire implémente une intelligence artificielle jouant au jeu de Nim contre un joueur humain avec difficulté adaptable. L'IA utilise une approche théorique basée sur la théorie des graphes et le calcul du noyau du graphe pour déterminer les coups gagnants. L'interface graphique SFML offre une expérience utilisateur fluide avec sélection visuelle des allumettes, animations et feedback en temps réel.

## Fonctionnement général du jeu de Nim

Le jeu de Nim classique consiste en un tas d'allumettes où les joueurs alternent pour en retirer 1 à *max_pick* allumettes. Le joueur qui prend la dernière allumette gagne. L'IA modélise le jeu comme un graphe orienté où :
- Chaque sommet représente un nombre d'allumettes restantes
- Les arêtes relient un état vers les états accessibles (en retirant 1 à *max_pick* allumettes)
- Le **noyau du graphe** identifie les positions perdantes pour l'adversaire
- L'IA priorise les transitions vers ces positions avec une probabilité contrôlée par le niveau de difficulté

## Architecture du projet

```
app
├── bin/              # Exécutable et fichiers de sortie
│   ├── nim_ai        # Binaire compilé
│   └── graphe.dot    # Représentation GraphViz (généré)
├── build/            # Fichiers de compilation CMake
├── graphs/           # Images de graphes générés
├── includes/         # En-têtes publics (.h/.hpp)
├── src/              # Sources d'implémentation (.c/.cpp)
├── CMakeLists.txt    # Configuration de build
└── main.cpp          # Point d'entrée
```

**Séparation des responsabilités :**
- **AI** : Calculs algorithmiques (graphes, noyau, stratégie)
- **Game** : État du jeu et règles métier
- **Interface** : Affichage SFML et interactions utilisateur
- **main** : Orchestration et cycle de vie

## Description des fichiers principaux

| Fichier | Langage | Rôle principal |
|---------|---------|----------------|
| `src/AI.c` | C | Implémentation de l'IA basée sur graphes |
| `includes/AI.h` | C | Déclarations des structures graphe/liste |
| `src/Game.c` | C | Gestion de l'état de jeu et règles |
| `includes/Game.h` | C | Interface C compatible C++ pour GameState |
| `src/Interface.cpp` | C++ | Interface graphique SFML complète |
| `includes/Interface.hpp` | C++ | Déclaration de l'interface utilisateur |
| `src/main.cpp` | C++ | Initialisation et boucle principale |
| `CMakeLists.txt` | CMake | Configuration multi-langage C/C++ + SFML |

## Fonctions et méthodes importantes

### Module AI (`AI.c`)
```c
graphe_t init_graphe(int nbRod, int maxPick);
```
Crée le graphe des positions du jeu avec arêtes sortantes vers positions accessibles.

```c
liste_t noyau(graphe_t *g);
```
Calcule le noyau du graphe (positions perdantes pour le joueur à bouger) via un algorithme rétrograde.

```c
int AI_pick(graphe_t *g, int node, int chances);
```
Sélectionne le coup optimal avec probabilité `chances%`, sinon coup aléatoire.

### Module Game (`Game.c`)
```c
void init_game(GameState* state);
```
Initialise l'état avec graphe IA et paramètres de difficulté.

```c
int ai_picks(GameState* state);
```
Exécute le tour de l'IA en appelant `AI_pick()`.

### Interface SFML (`Interface.cpp`)
```cpp
int run_interface(GameState& game_state);
```
Boucle principale SFML avec :
- Rendu des allumettes animées (wobble, hover, sélection)
- Gestion des clics (sélection, confirm/cancel/reset)
- Affichage des tours et résultats

## Instructions de compilation

```bash
# Prérequis : SFML 2.5+ (graphics, window, system)
# Ubuntu/Debian :
sudo apt install libsfml-dev cmake build-essential

# Création du répertoire de build
mkdir build && cd build

# Configuration CMake
cmake ..

# Compilation
make -j$(nproc)

# L'exécutable est généré dans bin/nim_ai
```

**Configuration par défaut :**
- 20 allumettes, max 3 par tour
- Difficulté IA : 50% (probabilité de coup optimal)

## Instructions d'exécution

```bash
# Depuis le répertoire build/
./nim_ai

# Ou depuis la racine du projet après build/
./bin/nim_ai
```

**Contrôles :**
- Clic gauche sur allumettes : sélection/désélection (max 3)
- Bouton CONFIRM : valider le tour
- Bouton CANCEL : désélectionner tout
- Bouton RESET : recommencer (20 allumettes)

## Organisation et conseils pour les développeurs

### Structure GameState
```c
typedef struct {
    int total_sticks;     // Allumettes restantes
    int max_pick;         // Maximum par tour
    int player_turn;      // 1=joueur, 0=IA
    int ai_difficulty;    // % coup optimal (0-100)
    graphe_t ai_graphe;   // État du graphe IA
} GameState;
```

### Bonnes pratiques adoptées
- **Interopérabilité C/C++** : `extern "C"` dans `Game.h`
- **Séparation stricte** : UI → Game → AI (aucun couplage circulaire)
- **Gestion mémoire** : `detruire_graphe()` appelée dans `main()`
- **Thread-safe** : IA purement fonctionnelle

### Extensions possibles
1. **Difficultés multiples** : Modifier `ai_difficulty` via menu
2. **Multi-tas Nim** : Étendre `graphe_t` en produit cartésien
3. **Replay système** : Enregistrer les coups dans `bin/replay.txt`
4. **Tests unitaires** : Ajouter répertoire `tests/` avec GoogleTest

### Dépannage compilation
```
Erreur SFML : sudo apt install libsfml-dev
Erreur CMake : rm -rf build/ && mkdir build && cd build && cmake ..
Police manquante : Interface fonctionne avec police système par défaut
```

**Licence** : Projet universitaire - libre usage interne au groupe.
