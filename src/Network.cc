#include "Network.h"
#include "ExactInference.h"
namespace bayesnet {
    Network::Network() : laplaceSmoothing(1), root(nullptr), features(vector<string>()), className(""), classNumStates(0) {}
    Network::Network(int smoothing) : laplaceSmoothing(smoothing), root(nullptr), features(vector<string>()), className(""), classNumStates(0) {}
    Network::Network(Network& other) : laplaceSmoothing(other.laplaceSmoothing), root(other.root), features(other.features), className(other.className), classNumStates(other.getClassNumStates())
    {
        for (auto& pair : other.nodes) {
            nodes[pair.first] = new Node(*pair.second);
        }
    }
    Network::~Network()
    {
        for (auto& pair : nodes) {
            delete pair.second;
        }
    }
    void Network::addNode(string name, int numStates)
    {
        if (nodes.find(name) != nodes.end()) {
            // if node exists update its number of states
            nodes[name]->setNumStates(numStates);
            return;
        }
        nodes[name] = new Node(name, numStates);
        if (root == nullptr) {
            root = nodes[name];
        }
    }
    vector<string> Network::getFeatures()
    {
        return features;
    }
    int Network::getClassNumStates()
    {
        return classNumStates;
    }
    string Network::getClassName()
    {
        return className;
    }
    void Network::setRoot(string name)
    {
        if (nodes.find(name) == nodes.end()) {
            throw invalid_argument("Node " + name + " does not exist");
        }
        root = nodes[name];
    }
    Node* Network::getRoot()
    {
        return root;
    }
    bool Network::isCyclic(const string& nodeId, unordered_set<string>& visited, unordered_set<string>& recStack)
    {
        if (visited.find(nodeId) == visited.end()) // if node hasn't been visited yet
        {
            visited.insert(nodeId);
            recStack.insert(nodeId);
            for (Node* child : nodes[nodeId]->getChildren()) {
                if (visited.find(child->getName()) == visited.end() && isCyclic(child->getName(), visited, recStack))
                    return true;
                else if (recStack.find(child->getName()) != recStack.end())
                    return true;
            }
        }
        recStack.erase(nodeId); // remove node from recursion stack before function ends
        return false;
    }
    void Network::addEdge(const string parent, const string child)
    {
        if (nodes.find(parent) == nodes.end()) {
            throw invalid_argument("Parent node " + parent + " does not exist");
        }
        if (nodes.find(child) == nodes.end()) {
            throw invalid_argument("Child node " + child + " does not exist");
        }
        // Temporarily add edge to check for cycles
        nodes[parent]->addChild(nodes[child]);
        nodes[child]->addParent(nodes[parent]);
        unordered_set<string> visited;
        unordered_set<string> recStack;
        if (isCyclic(nodes[child]->getName(), visited, recStack)) // if adding this edge forms a cycle
        {
            // remove problematic edge
            nodes[parent]->removeChild(nodes[child]);
            nodes[child]->removeParent(nodes[parent]);
            throw invalid_argument("Adding this edge forms a cycle in the graph.");
        }

    }
    map<string, Node*>& Network::getNodes()
    {
        return nodes;
    }
    void Network::fit(const vector<vector<int>>& dataset, const vector<int>& labels, const vector<string>& featureNames, const string& className)
    {
        features = featureNames;
        this->className = className;
        // Build dataset
        for (int i = 0; i < featureNames.size(); ++i) {
            this->dataset[featureNames[i]] = dataset[i];
        }
        this->dataset[className] = labels;
        this->classNumStates = *max_element(labels.begin(), labels.end()) + 1;
        estimateParameters();
    }

    void Network::estimateParameters()
    {
        auto dimensions = vector<int64_t>();
        for (auto [name, node] : nodes) {
            node->computeCPT(dataset, laplaceSmoothing);
        }
    }

    vector<int> Network::predict(const vector<vector<int>>& samples)
    {
        vector<int> predictions;
        vector<int> sample;
        for (int row = 0; row < samples[0].size(); ++row) {
            sample.clear();
            for (int col = 0; col < samples.size(); ++col) {
                sample.push_back(samples[col][row]);
            }
            predictions.push_back(predict_sample(sample).first);
        }
        return predictions;
    }
    vector<pair<int, double>> Network::predict_proba(const vector<vector<int>>& samples)
    {
        vector<pair<int, double>> predictions;
        vector<int> sample;
        for (int row = 0; row < samples[0].size(); ++row) {
            sample.clear();
            for (int col = 0; col < samples.size(); ++col) {
                sample.push_back(samples[col][row]);
            }
            predictions.push_back(predict_sample(sample));
        }
        return predictions;
    }
    double Network::score(const vector<vector<int>>& samples, const vector<int>& labels)
    {
        vector<int> y_pred = predict(samples);
        int correct = 0;
        for (int i = 0; i < y_pred.size(); ++i) {
            if (y_pred[i] == labels[i]) {
                correct++;
            }
        }
        return (double)correct / y_pred.size();
    }
    pair<int, double> Network::predict_sample(const vector<int>& sample)
    {
        // Ensure the sample size is equal to the number of features
        if (sample.size() != features.size()) {
            throw invalid_argument("Sample size (" + to_string(sample.size()) +
                ") does not match the number of features (" + to_string(features.size()) + ")");
        }
        auto inference = ExactInference(*this);
        map<string, int> evidence;
        for (int i = 0; i < sample.size(); ++i) {
            evidence[features[i]] = sample[i];
        }
        vector<double> classProbabilities = inference.variableElimination(evidence);

        // Find the class with the maximum posterior probability
        auto maxElem = max_element(classProbabilities.begin(), classProbabilities.end());
        int predictedClass = distance(classProbabilities.begin(), maxElem);
        double maxProbability = *maxElem;

        return make_pair(predictedClass, maxProbability);
    }
}