#include "../includes/Game.h"
#include <stdio.h>

void init_game(GameState* state) {
    state->ai_graphe = init_graphe(state->total_sticks, state->max_pick);
    printf("Jeu initialisé: %d allumettes (difficulte: %d)\n", 
           state->total_sticks, state->ai_difficulty);
}

int player_picks(GameState* state, int count) {
    state->total_sticks -= count;
    printf("Joueur prend %d. Restantes: %d\n", count, state->total_sticks);
    state->player_turn = 0;
    return count;
}

int ai_picks(GameState* state) {
    int pick = AI_pick(&state->ai_graphe, state->total_sticks, state->ai_difficulty);
    if (pick > state->max_pick) pick = state->max_pick;
    if (pick < 1) pick = 1;
    state->total_sticks -= pick;
    printf("IA (niveau %d) prend %d. Restantes: %d\n", 
           state->ai_difficulty, pick, state->total_sticks);
    state->player_turn = 1;
    return pick;
}

int is_game_over(const GameState* state) {
    return state->total_sticks <= 0;
}
