#ifndef PYCLASSIFIER_H
#define PYCLASSIFIER_H
#include "boost/python/detail/wrap_python.hpp"
#include <boost/python/numpy.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <torch/torch.h>
#include "PyWrap.h"
#include "Classifier.h"
#include "TypeId.h"

namespace pywrap {
    class PyClassifier : public bayesnet::BaseClassifier {
    public:
        PyClassifier(const std::string& module, const std::string& className, const bool sklearn = false);
        virtual ~PyClassifier();
        PyClassifier& fit(std::vector<std::vector<int>>& X, std::vector<int>& y, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states) override { return *this; };
        // X is nxm tensor, y is nx1 tensor
        PyClassifier& fit(torch::Tensor& X, torch::Tensor& y, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states) override;
        PyClassifier& fit(torch::Tensor& X, torch::Tensor& y);
        PyClassifier& fit(torch::Tensor& dataset, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states) override { return *this; };
        PyClassifier& fit(torch::Tensor& dataset, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const torch::Tensor& weights) override { return *this; };
        torch::Tensor predict(torch::Tensor& X) override;
        std::vector<int> predict(std::vector<std::vector<int >>& X) override { return std::vector<int>(); };
        float score(std::vector<std::vector<int>>& X, std::vector<int>& y) override { return 0.0; };
        float score(torch::Tensor& X, torch::Tensor& y) override;
        void setHyperparameters(nlohmann::json& hyperparameters) override;
        std::string version();
        std::string callMethodString(const std::string& method);
        std::string getVersion() override { return this->version(); };
        int getNumberOfNodes()const override { return 0; };
        int getNumberOfEdges()const override { return 0; };
        int getNumberOfStates() const override { return 0; };
        std::vector<std::string> show() const override { return std::vector<std::string>(); }
        std::vector<std::string> graph(const std::string& title = "") const override { return std::vector<std::string>(); }
        bayesnet::status_t getStatus() const override { return bayesnet::NORMAL; };
        std::vector<std::string> topological_order() override { return std::vector<std::string>(); }
        void dump_cpt() const override {};
    protected:
        void checkHyperparameters(const std::vector<std::string>& validKeys, const nlohmann::json& hyperparameters);
        nlohmann::json hyperparameters;
        void trainModel(const torch::Tensor& weights) override {};
    private:
        PyWrap* pyWrap;
        std::string module;
        std::string className;
        bool sklearn;
        clfId_t id;
        bool fitted;
    };
} /* namespace pywrap */
#endif /* PYCLASSIFIER_H */