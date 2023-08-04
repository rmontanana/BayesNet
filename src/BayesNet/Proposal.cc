#include "Proposal.h"
#include "ArffFiles.h"

namespace bayesnet {
    Proposal::Proposal(vector<vector<int>>& Xv_, vector<int>& yv_, vector<string>& features_, string& className_) : Xv(Xv_), yv(yv_), pFeatures(features_), pClassName(className_) {}
    Proposal::~Proposal()
    {
        for (auto& [key, value] : discretizers) {
            delete value;
        }
    }
    void Proposal::localDiscretizationProposal(map<string, vector<int>>& states, Network& model)
    {
        // order of local discretization is important. no good 0, 1, 2...
        // although we rediscretize features after the local discretization of every feature
        auto order = model.topological_sort();
        auto& nodes = model.getNodes();
        vector<int> indicesToReDiscretize;
        auto n_samples = Xf.size(1);
        bool upgrade = false; // Flag to check if we need to upgrade the model
        for (auto feature : order) {
            auto nodeParents = nodes[feature]->getParents();
            if (nodeParents.size() < 2) continue; // Only has class as parent
            upgrade = true;
            int index = find(pFeatures.begin(), pFeatures.end(), feature) - pFeatures.begin();
            indicesToReDiscretize.push_back(index); // We need to re-discretize this feature
            vector<string> parents;
            transform(nodeParents.begin(), nodeParents.end(), back_inserter(parents), [](const auto& p) { return p->getName(); });
            // Remove class as parent as it will be added later
            parents.erase(remove(parents.begin(), parents.end(), pClassName), parents.end());
            // Get the indices of the parents
            vector<int> indices;
            transform(parents.begin(), parents.end(), back_inserter(indices), [&](const auto& p) {return find(pFeatures.begin(), pFeatures.end(), p) - pFeatures.begin(); });
            // Now we fit the discretizer of the feature, conditioned on its parents and the class i.e. discretizer.fit(X[index], X[indices] + y)
            vector<string> yJoinParents;
            transform(yv.begin(), yv.end(), back_inserter(yJoinParents), [&](const auto& p) {return to_string(p); });
            for (auto idx : indices) {
                for (int i = 0; i < n_samples; ++i) {
                    yJoinParents[i] += to_string(Xv[idx][i]);
                }
            }
            auto arff = ArffFiles();
            auto yxv = arff.factorize(yJoinParents);
            auto xvf_ptr = Xf.index({ index }).data_ptr<float>();
            auto xvf = vector<mdlp::precision_t>(xvf_ptr, xvf_ptr + Xf.size(1));
            discretizers[feature]->fit(xvf, yxv);
            //
            //
            //
            auto tmp = discretizers[feature]->transform(xvf);
            Xv[index] = tmp;
            auto xStates = vector<int>(discretizers[pFeatures[index]]->getCutPoints().size() + 1);
            iota(xStates.begin(), xStates.end(), 0);
            //Update new states of the feature/node
            states[feature] = xStates;
        }
        // if (upgrade) {
        //     // Discretize again X (only the affected indices) with the new fitted discretizers
        //     for (auto index : indicesToReDiscretize) {
        //         auto Xt_ptr = Xf.index({ index }).data_ptr<float>();
        //         auto Xt = vector<float>(Xt_ptr, Xt_ptr + Xf.size(1));
        //         Xv[index] = discretizers[pFeatures[index]]->transform(Xt);
        //         auto xStates = vector<int>(discretizers[pFeatures[index]]->getCutPoints().size() + 1);
        //         iota(xStates.begin(), xStates.end(), 0);
        //         //Update new states of the feature/node
        //         states[pFeatures[index]] = xStates;
        //     }
        // }
    }
    void Proposal::fit_local_discretization(map<string, vector<int>>& states, torch::Tensor& y)
    {
        // Sharing Xv and yv with Classifier
        Xv = vector<vector<int>>();
        yv = vector<int>(y.data_ptr<int>(), y.data_ptr<int>() + y.size(0));
        // discretize input data by feature(row)
        for (int i = 0; i < pFeatures.size(); ++i) {
            auto* discretizer = new mdlp::CPPFImdlp();
            auto Xt_ptr = Xf.index({ i }).data_ptr<float>();
            auto Xt = vector<float>(Xt_ptr, Xt_ptr + Xf.size(1));
            discretizer->fit(Xt, yv);
            Xv.push_back(discretizer->transform(Xt));
            auto xStates = vector<int>(discretizer->getCutPoints().size() + 1);
            iota(xStates.begin(), xStates.end(), 0);
            states[pFeatures[i]] = xStates;
            discretizers[pFeatures[i]] = discretizer;
        }
        int n_classes = torch::max(y).item<int>() + 1;
        auto yStates = vector<int>(n_classes);
        iota(yStates.begin(), yStates.end(), 0);
        states[pClassName] = yStates;
    }
}