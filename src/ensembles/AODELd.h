#ifndef AODELD_H
#define AODELD_H
#include "Ensemble.h"
#include "classifiers/Proposal.h"
#include "classifiers/SPODELd.h"

namespace bayesnet {
    class AODELd : public Ensemble, public Proposal {
    public:
        AODELd(bool predict_voting = true);
        virtual ~AODELd() = default;
        AODELd& fit(torch::Tensor& X_, torch::Tensor& y_, const std::vector<std::string>& features_, const std::string& className_, map<std::string, std::vector<int>>& states_) override;
        void setHyperparameters(const nlohmann::json& hyperparameters) override;
        std::vector<std::string> graph(const std::string& name = "AODELd") const override;
    protected:
        void trainModel(const torch::Tensor& weights) override;
        void buildModel(const torch::Tensor& weights) override;
    };
}
#endif // !AODELD_H