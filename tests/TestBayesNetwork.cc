#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include "TestUtils.h"
#include "bayesnet/network/Network.h"

void buildModel(bayesnet::Network& net, const std::vector<std::string>& features, const std::string& className)
{
    std::vector<pair<int, int>> network = { {0, 1}, {0, 2}, {1, 3} };
    for (const auto& feature : features) {
        net.addNode(feature);
    }
    net.addNode(className);
    for (const auto& edge : network) {
        net.addEdge(features.at(edge.first), features.at(edge.second));
    }
    for (const auto& feature : features) {
        net.addEdge(className, feature);
    }
}

TEST_CASE("Test Bayesian Network", "[BayesNet]")
{

    auto raw = RawDatasets("iris", true);
    auto net = bayesnet::Network();
    double threshold = 1e-4;

    SECTION("Test get features")
    {
        net.addNode("A");
        net.addNode("B");
        REQUIRE(net.getFeatures() == std::vector<std::string>{"A", "B"});
        net.addNode("C");
        REQUIRE(net.getFeatures() == std::vector<std::string>{"A", "B", "C"});
    }
    SECTION("Test get edges")
    {
        net.addNode("A");
        net.addNode("B");
        net.addNode("C");
        net.addEdge("A", "B");
        net.addEdge("B", "C");
        REQUIRE(net.getEdges() == std::vector<pair<std::string, std::string>>{ {"A", "B"}, { "B", "C" } });
        REQUIRE(net.getNumEdges() == 2);
        net.addEdge("A", "C");
        REQUIRE(net.getEdges() == std::vector<pair<std::string, std::string>>{ {"A", "B"}, { "A", "C" }, { "B", "C" } });
        REQUIRE(net.getNumEdges() == 3);
    }
    SECTION("Test getNodes")
    {
        net.addNode("A");
        net.addNode("B");
        auto& nodes = net.getNodes();
        REQUIRE(nodes.count("A") == 1);
        REQUIRE(nodes.count("B") == 1);
    }

    SECTION("Test fit Network")
    {
        auto net2 = bayesnet::Network();
        auto net3 = bayesnet::Network();
        net3.initialize();
        net2.initialize();
        net.initialize();
        buildModel(net, raw.featuresv, raw.classNamev);
        buildModel(net2, raw.featurest, raw.classNamet);
        buildModel(net3, raw.featurest, raw.classNamet);
        std::vector<pair<std::string, std::string>> edges = {
            {"class", "sepallength"}, {"class", "sepalwidth"}, {"class", "petallength"},
            {"class", "petalwidth" }, {"sepallength", "sepalwidth"}, {"sepallength", "petallength"},
            {"sepalwidth", "petalwidth"}
        };
        REQUIRE(net.getEdges() == edges);
        REQUIRE(net2.getEdges() == edges);
        REQUIRE(net3.getEdges() == edges);
        std::vector<std::string> features = { "sepallength", "sepalwidth", "petallength", "petalwidth", "class" };
        REQUIRE(net.getFeatures() == features);
        REQUIRE(net2.getFeatures() == features);
        REQUIRE(net3.getFeatures() == features);
        auto& nodes = net.getNodes();
        auto& nodes2 = net2.getNodes();
        auto& nodes3 = net3.getNodes();
        // Check Nodes parents & children
        for (const auto& feature : features) {
            // Parents
            std::vector<std::string> parents, parents2, parents3, children, children2, children3;
            auto nodeParents = nodes[feature]->getParents();
            auto nodeParents2 = nodes2[feature]->getParents();
            auto nodeParents3 = nodes3[feature]->getParents();
            transform(nodeParents.begin(), nodeParents.end(), back_inserter(parents), [](const auto& p) { return p->getName(); });
            transform(nodeParents2.begin(), nodeParents2.end(), back_inserter(parents2), [](const auto& p) { return p->getName(); });
            transform(nodeParents3.begin(), nodeParents3.end(), back_inserter(parents3), [](const auto& p) { return p->getName(); });
            REQUIRE(parents == parents2);
            REQUIRE(parents == parents3);
            // Children
            auto nodeChildren = nodes[feature]->getChildren();
            auto nodeChildren2 = nodes2[feature]->getChildren();
            auto nodeChildren3 = nodes2[feature]->getChildren();
            transform(nodeChildren.begin(), nodeChildren.end(), back_inserter(children), [](const auto& p) { return p->getName(); });
            transform(nodeChildren2.begin(), nodeChildren2.end(), back_inserter(children2), [](const auto& p) { return p->getName(); });
            transform(nodeChildren3.begin(), nodeChildren3.end(), back_inserter(children3), [](const auto& p) { return p->getName(); });
            REQUIRE(children == children2);
            REQUIRE(children == children3);
        }
        // Fit networks
        net.fit(raw.Xv, raw.yv, raw.weightsv, raw.featuresv, raw.classNamev, raw.statesv);
        net2.fit(raw.dataset, raw.weights, raw.featurest, raw.classNamet, raw.statest);
        net3.fit(raw.Xt, raw.yt, raw.weights, raw.featurest, raw.classNamet, raw.statest);
        REQUIRE(net.getStates() == net2.getStates());
        REQUIRE(net.getStates() == net3.getStates());
        // Check Conditional Probabilities tables
        for (int i = 0; i < features.size(); ++i) {
            auto feature = features.at(i);
            for (const auto& feature : features) {
                auto cpt = nodes[feature]->getCPT();
                auto cpt2 = nodes2[feature]->getCPT();
                auto cpt3 = nodes3[feature]->getCPT();
                REQUIRE(cpt.equal(cpt2));
                REQUIRE(cpt.equal(cpt3));
            }
        }
    }
    SECTION("Test show")
    {
        auto net = bayesnet::Network();
        net.addNode("A");
        net.addNode("B");
        net.addNode("C");
        net.addEdge("A", "B");
        net.addEdge("A", "C");
        auto str = net.show();
        REQUIRE(str.size() == 3);
        REQUIRE(str[0] == "A -> B, C, ");
        REQUIRE(str[1] == "B -> ");
        REQUIRE(str[2] == "C -> ");
    }
    SECTION("Test topological_sort")
    {
        auto net = bayesnet::Network();
        net.addNode("A");
        net.addNode("B");
        net.addNode("C");
        net.addEdge("A", "B");
        net.addEdge("A", "C");
        auto sorted = net.topological_sort();
        REQUIRE(sorted.size() == 3);
        REQUIRE(sorted[0] == "A");
        bool result = sorted[1] == "B" && sorted[2] == "C";
        REQUIRE(result);
    }
    SECTION("Test graph")
    {
        auto net = bayesnet::Network();
        net.addNode("A");
        net.addNode("B");
        net.addNode("C");
        net.addEdge("A", "B");
        net.addEdge("A", "C");
        auto str = net.graph("Test Graph");
        REQUIRE(str.size() == 7);
        REQUIRE(str[0] == "digraph BayesNet {\nlabel=<BayesNet Test Graph>\nfontsize=30\nfontcolor=blue\nlabelloc=t\nlayout=circo\n");
        REQUIRE(str[1] == "A [shape=circle] \n");
        REQUIRE(str[2] == "A -> B");
        REQUIRE(str[3] == "A -> C");
        REQUIRE(str[4] == "B [shape=circle] \n");
        REQUIRE(str[5] == "C [shape=circle] \n");
        REQUIRE(str[6] == "}\n");
    }
    SECTION("Test predict")
    {
        auto net = bayesnet::Network();
        buildModel(net, raw.featuresv, raw.classNamev);
        net.fit(raw.Xv, raw.yv, raw.weightsv, raw.featuresv, raw.classNamev, raw.statesv);
        std::vector<std::vector<int>> test = { {1, 2, 0, 1, 1}, {0, 1, 2, 0, 1}, {0, 0, 0, 0, 1}, {2, 2, 2, 2, 1} };
        std::vector<int> y_test = { 2, 2, 0, 2, 1 };
        auto y_pred = net.predict(test);
        REQUIRE(y_pred == y_test);
    }
    SECTION("Test predict_proba")
    {
        auto net = bayesnet::Network();
        buildModel(net, raw.featuresv, raw.classNamev);
        net.fit(raw.Xv, raw.yv, raw.weightsv, raw.featuresv, raw.classNamev, raw.statesv);
        std::vector<std::vector<int>> test = { {1, 2, 0, 1, 1}, {0, 1, 2, 0, 1}, {0, 0, 0, 0, 1}, {2, 2, 2, 2, 1} };
        std::vector<std::vector<double>> y_test = {
            {0.450237, 0.0866621, 0.463101},
            {0.244443, 0.0925922, 0.662964},
            {0.913441, 0.0125857, 0.0739732},
            {0.450237, 0.0866621, 0.463101},
            {0.0135226, 0.971726, 0.0147519}
        };
        auto y_pred = net.predict_proba(test);
        REQUIRE(y_pred.size() == 5);
        REQUIRE(y_pred[0].size() == 3);
        for (int i = 0; i < y_pred.size(); ++i) {
            for (int j = 0; j < y_pred[i].size(); ++j) {
                REQUIRE(y_pred[i][j] == Catch::Approx(y_test[i][j]).margin(threshold));
            }
        }
    }
    SECTION("Test score")
    {
        auto net = bayesnet::Network();
        buildModel(net, raw.featuresv, raw.classNamev);
        net.fit(raw.Xv, raw.yv, raw.weightsv, raw.featuresv, raw.classNamev, raw.statesv);
        auto score = net.score(raw.Xv, raw.yv);
        REQUIRE(score == Catch::Approx(0.97333333).margin(threshold));
    }
}