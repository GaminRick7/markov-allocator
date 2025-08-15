#include "MarkovPredictor.h"

MarkovPredictor::MarkovPredictor() 
    : count(Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE>::Zero()),
      transition(Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE>::Zero()) {}

void MarkovPredictor::update_count(int old_state, int new_state) {
    if (old_state >= 0 && old_state < MATRIX_SIZE && new_state >= 0 && new_state < MATRIX_SIZE) {
        count(old_state, new_state)++;
    }
}

void MarkovPredictor::update_transition(int row) {
    if (row < 0 || row >= MATRIX_SIZE) return;
    
    float sum = 0;
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        sum += count(row, i);
    }
    
    if (sum > 0) {
        for (int i = 0; i < MATRIX_SIZE; ++i) {
            transition(row, i) = count(row, i) / sum;
        }
    }
}

void MarkovPredictor::update(int from, int to) {
    update_count(from, to);
    update_transition(from);
}

int MarkovPredictor::predict(int from) const {
    if (from < 0 || from >= MATRIX_SIZE) return from;
    
    // Create current state vector
    Eigen::Matrix<float, 1, MATRIX_SIZE> cur = 
        Eigen::Matrix<float, 1, MATRIX_SIZE>::Zero();
    cur(0, from) = 1;
    
    // Calculate next state probabilities
    Eigen::Matrix<float, 1, MATRIX_SIZE> next = cur * transition;
    
    // Find state with maximum probability
    int max_state = from;
    float max_val = 0;
    
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        if (next(i) > max_val) {
            max_val = next(i);
            max_state = i;
        }
    }
    
    return max_state;
}

