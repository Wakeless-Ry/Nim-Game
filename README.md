# NimAI – Jeu de Nim avec Intelligence Artificielle

Projet universitaire implémentant le jeu de **Nim** avec une IA adversaire basée sur la théorie des graphes. L'IA utilise un graphe d'états pour calculer des coups optimaux et peut jouer à différents niveaux de difficulté.

## 1. Objectif du projet

Étudier les algorithmes d'exploration de graphes pour modéliser le jeu de Nim :
- Chaque nœud représente un état du jeu (nombre d'allumettes restantes).
- Les arêtes représentent les coups possibles (retrait de 1 à maxPick allumettes).
- L'IA identifie les positions gagnantes (noyaux du graphe) et propose des coups optimaux.

**Fonction clé** : `AI_pick(graphe_t *g, int node, int chances)` retourne le coup de l'IA :
- `node` : état actuel (nombre d'allumettes restantes)
- `chances` : niveau de difficulté (25=facile, 50=normal, 75=difficile, 100=impossible)
- Retourne le nombre d'allumettes à retirer (différence entre node et coup optimal)

## 2. Structure du projet

```
.
├── bin/              # Exécutable compilé (nim_ai)
├── includes/         # Fichiers d'en-tête (futurs .h)
├── src/
│   └── Nim.c         # Code principal : graphe, IA, main()
└── CMakeLists.txt    # Configuration de build CMake
```

## 3. Prérequis

- **CMake** ≥ 3.15
- **Compilateur C** : GCC, Clang (Linux/macOS) ou MSVC (Windows)
- **Optionnel** : Qt Creator, Graphviz (pour visualiser graphe.dot)

**Installation rapide (Linux/macOS)** :
```bash
# Ubuntu/Debian
sudo apt install cmake gcc graphviz

# macOS (Homebrew)
brew install cmake gcc graphviz
```

## 4. Compilation

### En ligne de commande

```bash
# Depuis la racine du projet
mkdir -p build && cd build
cmake ..
cmake --build . --parallel
```

**Résultat** : `bin/nim_ai` est créé.

## 5. Exécution

```bash
../bin/nim_ai 
```

**Sortie** :
- Affiche le plan de jeu optimal pour chaque état (S0 à S19)
- Génère `graphe.dot` (visualisable avec Graphviz)

**Visualisation du graphe** :
```bash
dot -Tpng graphe.dot -o ../graphs/graphe.png
```

## 6. Architecture du code

### Structures principales (Nim.c)

```c
typedef struct chainon { int numero_sommet; struct chainon* next; } chainon_t;
typedef chainon_t* liste_t;
typedef struct { int nbr_sommets; liste_t *listes; } graphe_t;
```

### Fonctions clés

| Fonction | Description |
|----------|-------------|
| `init_graphe(nbRod, maxPick)` | Crée le graphe des états du jeu |
| `noyau(g)` | Calcule les positions gagnantes (noyaux) |
| `jouer_coup(g, sommet)` | Coup optimal depuis un état donné |
| `AI_pick(g, node, chances)` | Coup IA (optimal ou aléatoire selon chances) |
| `write_graphviz(f, g)` | Export Graphviz (.dot) |

## 7. Intégration dans un jeu complet

Pour utiliser l'IA dans votre jeu Nim complet :

```c
graphe_t g = init_graphe(20, 3);   // 20 allumettes, max 3 par coup
int etat_actuel = 15;              // 15 allumettes restantes
int coup_ia = AI_pick(&g, etat_actuel, 75);  // Difficulté difficile
int allumettes_retriees = etat_actuel - coup_ia;
detruire_graphe(&g);
```

## 8. Contribution

1. Ajoutez vos `.c` dans `src/` et mettez à jour `SOURCES` dans CMakeLists.txt
2. Placez les `.h` dans `includes/`
3. Testez avec `cmake --build build --clean-first`
4. Documentez vos ajouts dans ce README

## 9. Licence

Projet universitaire – utilisation libre dans le cadre du cours.