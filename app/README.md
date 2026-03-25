# NimAI — Jeu des Allumettes contre une IA

Application graphique de jeu de Nim (joueur humain contre une intelligence artificielle) implémentée en C/C++ avec SFML. Le projet explore plusieurs algorithmes de décision pour l'IA, chacun modélisant un niveau de "rationalité" différent — de l'imitation pure à la résolution mathématiquement parfaite du jeu.

---

## Règles du jeu

Le **jeu de Nim** (ou jeu des allumettes) se joue sur un tas d'allumettes :

- À chaque tour, le joueur actif prend **1, 2 ou 3 allumettes** dans le tas.
- Le joueur qui prend **la dernière allumette gagne** la partie.
- Les deux joueurs alternent : le joueur humain commence.

### Solution mathématique

Le Nim avec `max_pick = 3` est un jeu parfaitement résolu. Toute position peut être classée en :

- **P-position** (Position Perdante) : le joueur à qui c'est le tour **perd** face à un adversaire optimal.
- **N-position** (Position Gagnante) : le joueur à qui c'est le tour **gagne** avec le bon coup.

La règle est simple : une position `n` est une P-position si et seulement si `n mod 4 == 0`.

```
n  = 0  → P  (plus d'allumettes, le tour précédent a pris la dernière)
n  = 4  → P  (tout coup laisse 1, 2 ou 3 : toutes des N-positions pour l'adversaire)
n  = 8  → P
n  = 12 → P
...
```

La stratégie optimale : toujours ramener le total à un multiple de 4. Par exemple depuis 10 allumettes, prendre 2 (laisse 8 = P-position). Depuis une P-position, tout coup mène à une N-position : il est **impossible de gagner** face à un adversaire parfait si on est en P-position.

---

## Installation et lancement

### Prérequis

- Linux (Ubuntu/Debian recommandé)
- `cmake` ≥ 3.16, `build-essential`, `libsfml-dev`

### Lancement rapide

```bash
cd app
bash run.sh          # installe les dépendances manquantes, compile et lance
bash run.sh --clean  # recompile depuis zéro
```

### Build manuel

```bash
cd app
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..
./bin/nim_ai
```

---

## Interface

### Menu principal

Au lancement, le menu propose trois paramètres :

| Section | Options |
|---------|---------|
| **Stratégie de l'IA** | GRAPHE, COPIE, MINIMAX, MCTS, RECUIT |
| **Difficulté** | FACILE (25), MOYEN (50), DIFFICILE (75), IMPOSSIBLE (100) |
| **Nombre d'allumettes** | 3 à 50 |

La signification concrète de la difficulté dépend de la stratégie choisie (voir ci-dessous).

### Déroulement d'une partie

1. Cliquer sur les allumettes pour en sélectionner 1, 2 ou 3.
2. Appuyer sur **CONFIRMER** pour valider son coup.
3. L'IA joue immédiatement après.
4. La partie se termine quand plus aucune allumette ne reste.
5. En fin de partie : **REJOUER** (mêmes paramètres) ou **MENU** (retour aux réglages).

---

## Les 5 stratégies de l'IA

### 1. GRAPHE OPTIMALE

**Principe :** À chaque tour, l'IA tire un nombre aléatoire. Avec une probabilité égale à `difficulté %`, elle joue le coup mathématiquement optimal (via le calcul du noyau du graphe). Sinon, elle joue un coup aléatoire entre 1 et 3.

```
P(coup optimal) = ai_difficulty / 100
```

**Mapping difficulté :**

| Niveau | Valeur | Comportement |
|--------|--------|--------------|
| FACILE | 25 | Optimal 1 fois sur 4 |
| MOYEN | 50 | Optimal 1 fois sur 2 |
| DIFFICILE | 75 | Optimal 3 fois sur 4 |
| IMPOSSIBLE | 100 | Toujours optimal |

**Efficacité :** La stratégie GRAPHE OPTIMALE est la plus "propre" pour modéliser un joueur imparfait. À IMPOSSIBLE, elle est **mathématiquement parfaite** — elle ne peut pas perdre face à un joueur en N-position. Aux niveaux inférieurs, les erreurs sont indépendantes et uniformément distribuées, ce qui lui donne un caractère aléatoire mais prévisible.

---

### 2. COPIE — Imitation

**Principe :** L'IA rejoue exactement le nombre d'allumettes que le joueur humain vient de prendre. Au premier tour (pas encore de coup du joueur), elle prend 1.

```
AI_pick = last_player_pick  (ou 1 si premier tour)
```

**Mapping difficulté :** La difficulté n'a **aucun effet** sur cette stratégie.

**Efficacité :** La stratégie COPIE est la plus faible de toutes et n'a aucune base en théorie des jeux. Elle peut par hasard copier un bon coup si le joueur joue lui-même correctement, mais elle ignore totalement l'état du jeu. En particulier :

- Elle ne s'adapte jamais à la parité des positions.
- Si le joueur joue 3 et qu'il reste 4 allumettes, COPIE prendra 3, laissant 1 → le joueur gagne.
- Elle peut dépasser le nombre d'allumettes restantes (cas géré par clamping à max_pick).

En pratique, COPIE se comporte comme un joueur quasi-aléatoire avec un léger biais vers le dernier coup joué.

---

### 3. MINIMAX — Negamax à profondeur bornée

**Principe :** L'IA explore l'arbre de jeu jusqu'à une profondeur limitée via l'algorithme **Negamax** (variante de Minimax à somme nulle). Chaque position est évaluée à `+1` (gagne), `-1` (perd) ou `0` (inconnu à l'horizon). En cas d'égalité entre plusieurs coups, le choix est uniformisé par réservoir sampling.

**Mapping difficulté :**

```
depth = ai_difficulty / 10
```

| Niveau | Valeur | Profondeur | Comportement |
|--------|--------|------------|--------------|
| FACILE | 25 | 2 | Voit 2 coups à l'avance → quasi-aléatoire |
| MOYEN | 50 | 5 | Vision partielle |
| DIFFICILE | 75 | 7 | Très solide |
| IMPOSSIBLE | 100 | 10 | Optimal pour toute partie ≤ 50 allumettes |

**Efficacité :** C'est la stratégie la plus "rigoureuse" dans sa montée en puissance. À `depth ≥ max_pick + 1 = 4`, l'IA voit toujours au moins un cycle complet de positions et joue **mathématiquement parfaitement**. À partir de DIFFICILE (depth = 7), aucune erreur n'est possible pour des parties standards. À FACILE (depth = 2), l'IA ne voit que deux coups et peut manquer des pièges distants.

---

### 4. MCTS — Monte Carlo Tree Search

**Principe :** Plutôt que d'explorer l'arbre de manière exhaustive, l'IA **simule des parties aléatoires complètes** depuis chaque coup candidat. Les simulations sont distribuées en round-robin entre les coups disponibles. Le coup avec le meilleur taux de victoire estimé est sélectionné.

**Mapping difficulté :**

```
nb_simulations = ai_difficulty + 1
```

| Niveau | Valeur | Simulations | Comportement |
|--------|--------|-------------|--------------|
| FACILE | 25 | 26 | Estimation très bruitée |
| MOYEN | 50 | 51 | Partiel |
| DIFFICILE | 75 | 76 | Bon sur petits tas |
| IMPOSSIBLE | 100 | 101 | Quasi-optimal pour N ≤ ~20 |

**Efficacité :** Le MCTS est fondamentalement **statistique** : plus le budget de simulations est grand, plus l'estimation est précise. Avec 101 simulations et seulement 2-3 coups possibles (~34 simulations par coup), la convergence est bonne pour de petits tas mais devient insuffisante pour de grands N (50 allumettes). Contrairement à MINIMAX, le MCTS peut-être battu même à IMPOSSIBLE si la variance des simulations joue contre lui. Il reste néanmoins très efficace en pratique pour les tailles standard (10-20 allumettes).

---

### 5. RECUIT — Recuit Simulé (Simulated Annealing)

**Principe :** L'IA calcule d'abord le coup optimal (via le noyau du graphe), puis tire un coup candidat aléatoire. Elle compare les deux via une évaluation Negamax à profondeur 2. Si le candidat est meilleur ou équivalent, il est accepté. Sinon, il est accepté avec une probabilité décroissante avec l'écart de qualité :

```
P(accepter sous-optimal) = e^(-Δ/T)
```

où `Δ` est la perte de qualité et `T` la température.

**Mapping difficulté :**

```
T = (100 - ai_difficulty) / 20.0 + 0.01
```

| Niveau | Valeur | Température | Comportement |
|--------|--------|-------------|--------------|
| FACILE | 25 | ≈ 3.76 | Accepte fréquemment les mauvais coups |
| MOYEN | 50 | ≈ 2.51 | Mixte |
| DIFFICILE | 75 | ≈ 1.26 | Rarement sous-optimal |
| IMPOSSIBLE | 100 | ≈ 0.01 | Quasi-toujours optimal |

**Efficacité :** Le recuit simulé est la stratégie la plus **originale** du point de vue du comportement. À haute température, l'IA joue de façon erratique mais pas complètement aléatoire — elle a une légère tendance vers les bons coups. À basse température, elle converge vers l'optimal. L'avantage conceptuel est que les erreurs ne sont pas uniformes : les coups "à peine sous-optimaux" sont bien plus souvent acceptés que les catastrophes. En pratique, elle est légèrement moins fiable que MINIMAX à difficulté équivalente car elle ne garantit pas la convergence à faible budget.

---

## Analyse comparative

### Efficacité à IMPOSSIBLE

| Stratégie | Garantie théorique | Notes |
|-----------|-------------------|-------|
| GRAPHE OPTIMAL | Parfaite | Joue toujours dans le noyau |
| COPIE | Nulle | Aucune base théorique |
| MINIMAX | Parfaite (depth ≥ 4) | Couvre tous les cas en depth=10 |
| MCTS | Quasi-parfaite | Variance résiduelle avec 101 simulations |
| RECUIT | Quasi-parfaite | T=0.01 → P(erreur) ≈ e^(-1/0.01) ≈ 0 |

### Pour simuler un joueur "humain imparfait"

La stratégie **GRAPHE OPTIMAL** est la plus fidèle : les erreurs sont indépendantes et proportionnelles au niveau choisi. **RECUIT** simule un joueur qui fait moins souvent de grosses erreurs que de petites — comportement plus "humain". **COPIE** simule un joueur distrait qui ne réfléchit pas. **MINIMAX FACILE** simule un joueur qui voit loin devant lui mais pas assez loin.

### Limite universelle : les P-positions

Aucune stratégie ne peut gagner contre un joueur parfait si elle se retrouve en P-position (multiple de 4 restant). La question de l'efficacité se pose donc surtout quand l'IA commence en N-position (ce qui est le cas si le nombre initial d'allumettes n'est pas un multiple de 4, puisque le joueur humain commence).

---

## Architecture technique

```
app/
├── src/
│   ├── AI.c          — Graphe d'états, noyau (P-positions), 5 stratégies IA
│   ├── Game.c        — Gestion des tours (player_picks, ai_picks)
│   ├── Interface.cpp — Interface graphique SFML (sélection d'allumettes, animations)
│   ├── menu.cpp      — Menu SFML (stratégie, difficulté, allumettes)
│   └── main.cpp      — Boucle principale (menu → jeu → menu)
└── includes/
    ├── AI.h          — GameState, AIStrategyType, déclarations
    └── Game.h        — Interface C/C++
```

**Principe du graphe d'états :** L'IA construit un graphe orienté où chaque nœud `i` représente `i` allumettes restantes, et les arêtes `i → i-j` représentent les coups valides. Le **noyau** du graphe (positions sans successeur dans le noyau) correspond exactement aux P-positions `{0, 4, 8, 12, ...}`.

**Technologies :**
- Logique de jeu et IA en **C99** (performance, gestion manuelle de la mémoire)
- Interface graphique en **C++17** avec **SFML 2.5**
- Build : **CMake 3.16+**
