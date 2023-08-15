#include "AODE.h"

namespace bayesnet {
    AODE::AODE() : Ensemble() {}
    void AODE::buildModel(const torch::Tensor& weights)
    {
        models.clear();
        for (int i = 0; i < features.size(); ++i) {
            models.push_back(std::make_unique<SPODE>(i));
        }
    }
    vector<string> AODE::graph(const string& title) const
    {
        return Ensemble::graph(title);
    }
}