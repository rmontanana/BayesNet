#include "Ensemble.h"

namespace bayesnet {

    Ensemble::Ensemble() : Classifier(Network()), n_models(0) {}

    void Ensemble::trainModel(const torch::Tensor& weights)
    {
        n_models = models.size();
        for (auto i = 0; i < n_models; ++i) {
            // fit with std::vectors
            models[i]->fit(dataset, features, className, states);
        }
    }
    std::vector<int> Ensemble::voting(torch::Tensor& y_pred)
    {
        auto y_pred_ = y_pred.accessor<int, 2>();
        std::vector<int> y_pred_final;
        int numClasses = states.at(className).size();
        // y_pred is m x n_models with the prediction of every model for each sample
        for (int i = 0; i < y_pred.size(0); ++i) {
            // votes store in each index (value of class) the significance added by each model
            // i.e. votes[0] contains how much value has the value 0 of class. That value is generated by the models predictions
            std::vector<double> votes(numClasses, 0.0);
            for (int j = 0; j < n_models; ++j) {
                votes[y_pred_[i][j]] += significanceModels.at(j);
            }
            // argsort in descending order
            auto indices = argsort(votes);
            y_pred_final.push_back(indices[0]);
        }
        return y_pred_final;
    }
    torch::Tensor Ensemble::predict(torch::Tensor& X)
    {
        if (!fitted) {
            throw std::logic_error("Ensemble has not been fitted");
        }
        torch::Tensor y_pred = torch::zeros({ X.size(1), n_models }, torch::kInt32);
        auto threads{ std::vector<std::thread>() };
        std::mutex mtx;
        for (auto i = 0; i < n_models; ++i) {
            threads.push_back(std::thread([&, i]() {
                auto ypredict = models[i]->predict(X);
                std::lock_guard<std::mutex> lock(mtx);
                y_pred.index_put_({ "...", i }, ypredict);
                }));
        }
        for (auto& thread : threads) {
            thread.join();
        }
        return torch::tensor(voting(y_pred));
    }
    std::vector<int> Ensemble::predict(std::vector<std::vector<int>>& X)
    {
        if (!fitted) {
            throw std::logic_error("Ensemble has not been fitted");
        }
        long m_ = X[0].size();
        long n_ = X.size();
        std::vector<std::vector<int>> Xd(n_, std::vector<int>(m_, 0));
        for (auto i = 0; i < n_; i++) {
            Xd[i] = std::vector<int>(X[i].begin(), X[i].end());
        }
        torch::Tensor y_pred = torch::zeros({ m_, n_models }, torch::kInt32);
        for (auto i = 0; i < n_models; ++i) {
            y_pred.index_put_({ "...", i }, torch::tensor(models[i]->predict(Xd), torch::kInt32));
        }
        return voting(y_pred);
    }
    float Ensemble::score(torch::Tensor& X, torch::Tensor& y)
    {
        if (!fitted) {
            throw std::logic_error("Ensemble has not been fitted");
        }
        auto y_pred = predict(X);
        int correct = 0;
        for (int i = 0; i < y_pred.size(0); ++i) {
            if (y_pred[i].item<int>() == y[i].item<int>()) {
                correct++;
            }
        }
        return (double)correct / y_pred.size(0);
    }
    float Ensemble::score(std::vector<std::vector<int>>& X, std::vector<int>& y)
    {
        if (!fitted) {
            throw std::logic_error("Ensemble has not been fitted");
        }
        auto y_pred = predict(X);
        int correct = 0;
        for (int i = 0; i < y_pred.size(); ++i) {
            if (y_pred[i] == y[i]) {
                correct++;
            }
        }
        return (double)correct / y_pred.size();
    }
    std::vector<std::string> Ensemble::show() const
    {
        auto result = std::vector<std::string>();
        for (auto i = 0; i < n_models; ++i) {
            auto res = models[i]->show();
            result.insert(result.end(), res.begin(), res.end());
        }
        return result;
    }
    std::vector<std::string> Ensemble::graph(const std::string& title) const
    {
        auto result = std::vector<std::string>();
        for (auto i = 0; i < n_models; ++i) {
            auto res = models[i]->graph(title + "_" + std::to_string(i));
            result.insert(result.end(), res.begin(), res.end());
        }
        return result;
    }
    int Ensemble::getNumberOfNodes() const
    {
        int nodes = 0;
        for (auto i = 0; i < n_models; ++i) {
            nodes += models[i]->getNumberOfNodes();
        }
        return nodes;
    }
    int Ensemble::getNumberOfEdges() const
    {
        int edges = 0;
        for (auto i = 0; i < n_models; ++i) {
            edges += models[i]->getNumberOfEdges();
        }
        return edges;
    }
    int Ensemble::getNumberOfStates() const
    {
        int nstates = 0;
        for (auto i = 0; i < n_models; ++i) {
            nstates += models[i]->getNumberOfStates();
        }
        return nstates;
    }
}