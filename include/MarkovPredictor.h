#pragma once

#include <Eigen/Dense>

class MarkovPredictor {
public:
    MarkovPredictor();
    void update(int from, int to);
    int predict(int from) const;

private:
    static constexpr int MATRIX_SIZE = 8;  // Match markov-allocator-master
    Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE> count;
    Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE> transition;
    
    void update_count(int old_state, int new_state);
    void update_transition(int row);
};

