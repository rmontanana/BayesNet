#ifndef STATISTICS_H
#define STATISTICS_H
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace platform {
    struct WTL {
        int win;
        int tie;
        int loss;
    };
    struct FriedmanResult {
        double statistic;
        double criticalValue;
        long double pvalue;
        bool reject;
    };
    struct HolmLine {
        string model;
        long double pvalue;
        double rank;
        WTL wtl;
        bool reject;
    };
    struct HolmResult {
        string model;
        vector<HolmLine> holmLines;
    };
    class Statistics {
    public:
        Statistics(vector<string>& models, vector<string>& datasets, json data, double significance = 0.05, bool output = true);
        bool friedmanTest();
        void postHocHolmTest(bool friedmanResult);
        FriedmanResult& getFriedmanResult();
        HolmResult& getHolmResult();
    private:
        void fit();
        void computeRanks();
        void computeWTL();
        vector<string> models;
        vector<string> datasets;
        json data;
        double significance;
        bool output;
        bool fitted = false;
        int nModels = 0;
        int nDatasets = 0;
        int controlIdx = 0;
        map<int, WTL> wtl;
        map<string, float> ranks;
        int maxModelName = 0;
        int maxDatasetName = 0;
        FriedmanResult friedmanResult;
        HolmResult holmResult;
    };
}
#endif // !STATISTICS_H