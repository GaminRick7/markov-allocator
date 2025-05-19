#pragma once

class MarkovPredictor {
public:
    MarkovPredictor();
    void update(int from, int to);
    int predict(int from) const;

private:
    static constexpr int NUM_STATES = 32;
    int transition[NUM_STATES][NUM_STATES] = {};
};

