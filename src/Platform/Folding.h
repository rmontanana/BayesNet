#ifndef FOLDING_H
#define FOLDING_H
#include <vector>
using namespace std;

class Fold {
protected:
    int k;
    int n;
    int seed;
public:
    Fold(int k, int n, int seed = -1) : k(k), n(n), seed(seed) {}
    virtual pair<vector<int>, vector<int>> getFold(int nFold) = 0;
    virtual ~Fold() = default;
};
class KFold : public Fold {
private:
    vector<int> indices;
public:
    KFold(int k, int n, int seed = -1);
    pair<vector<int>, vector<int>> getFold(int nFold);
};
class StratifiedKFold : public Fold {
    vector<vector<int>> stratified_indices;
public:
    StratifiedKFold(int k, const vector<int>& y, int seed = -1);
    pair<vector<int>, vector<int>> getFold(int nFold);
};
#endif