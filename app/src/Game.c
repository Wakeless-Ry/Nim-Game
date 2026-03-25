#include "../includes/Game.h"
#include <stdio.h>

void init_game(GameState *state) {
  state->ai_graphe = init_graphe(state->total_sticks, state->max_pick);
  state->last_pick = -1;
  printf("Jeu initialise: %d allumettes (difficulte: %d)\n",
         state->total_sticks, state->ai_difficulty);
  FILE *f = fopen("graphs/graphe.dot", "w");
  if (f) {
    write_graphviz(f, &state->ai_graphe);
    fclose(f);
  }
}

int player_picks(GameState *state, int count) {
  state->total_sticks -= count;
  printf("Le joueur prend %d. Restantes: %d\n", count, state->total_sticks);
  state->player_turn = 0;
  state->last_pick = count;
  return count;
}

int ai_picks(GameState *state) {
  int pick = AI_pick(&state->ai_graphe, state->total_sticks, state);
  if (pick > state->max_pick)
    pick = state->max_pick;
  if (pick < 1)
    pick = 1;
  state->total_sticks -= pick;
  printf("L'IA (niveau %d) prend %d. Restantes: %d\n", state->ai_difficulty,
         pick, state->total_sticks);
  state->player_turn = 1;
  state->last_pick = pick;
  return pick;
}

int is_game_over(const GameState *state) { return state->total_sticks <= 0; }
