#include "MarkovPredictor.h"

MarkovPredictor::MarkovPredictor() {}

void MarkovPredictor::update(int from, int to) {
    if (from >= 0 && from < NUM_STATES && to >= 0 && to < NUM_STATES) {
        transition[from][to]++;
    }
}

int MarkovPredictor::predict(int from) const {
    if (from < 0 || from >= NUM_STATES) return from;
    int best = from;
    int max_count = 0;
    for (int to = 0; to < NUM_STATES; ++to) {
        if (transition[from][to] > max_count) {
            max_count = transition[from][to];
            best = to;
        }
    }
    return best;
}

