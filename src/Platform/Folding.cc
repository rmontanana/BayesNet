#include "Folding.h"
#include <algorithm>
#include <map>
#include <random>

using namespace std;

KFold::KFold(int k, int n, int seed) : k(k), n(n), seed(seed)
{
    indices = vector<int>(n);
    iota(begin(indices), end(indices), 0); // fill with 0, 1, ..., n - 1
    random_device rd;
    default_random_engine random_seed(seed == -1 ? rd() : seed);
    shuffle(indices.begin(), indices.end(), random_seed);
}
pair<vector<int>, vector<int>> KFold::getFold(int nFold)
{

    if (nFold >= k || nFold < 0) {
        throw out_of_range("nFold (" + to_string(nFold) + ") must be less than k (" + to_string(k) + ")");
    }
    int nTest = n / k;
    auto train = vector<int>();
    auto test = vector<int>();
    for (int i = 0; i < n; i++) {
        if (i >= nTest * nFold && i < nTest * (nFold + 1)) {
            test.push_back(indices[i]);
        } else {
            train.push_back(indices[i]);
        }
    }
    return { train, test };
}
StratifiedKFold::StratifiedKFold(int k, const vector<int>& y, int seed) :
    k(k), seed(seed)
{
    n = y.size();
    stratified_indices = vector<vector<int>>(k);
    int fold_size = n / k;
    int remainder = n % k;
    // Compute class counts and indices
    auto class_indices = map<int, vector<int>>();
    vector<int> class_counts(*max_element(y.begin(), y.end()) + 1, 0);
    for (auto i = 0; i < n; ++i) {
        class_counts[y[i]]++;
        class_indices[y[i]].push_back(i);
    }
    // Shuffle class indices
    random_device rd;
    default_random_engine random_seed(seed == -1 ? rd() : seed);
    for (auto& [cls, indices] : class_indices) {
        shuffle(indices.begin(), indices.end(), random_seed);
    }
    // Assign indices to folds
    for (auto label = 0; label < class_counts.size(); ++label) {
        auto num_samples_to_take = class_counts[label] / k;
        if (num_samples_to_take == 0)
            continue;
        auto remainder_samples_to_take = class_counts[label] % k;
        for (auto fold = 0; fold < k; ++fold) {
            auto it = next(class_indices[label].begin(), num_samples_to_take);
            move(class_indices[label].begin(), it, back_inserter(stratified_indices[fold]));  // ##
            class_indices[label].erase(class_indices[label].begin(), it);
        }
        while (remainder_samples_to_take > 0) {
            int fold = (arc4random() % static_cast<int>(k));
            if (stratified_indices[fold].size() == fold_size) {
                continue;
            }
            auto it = next(class_indices[label].begin(), 1);
            stratified_indices[fold].push_back(*class_indices[label].begin());
            class_indices[label].erase(class_indices[label].begin(), it);
            remainder_samples_to_take--;
        }
    }
}
pair<vector<int>, vector<int>> StratifiedKFold::getFold(int nFold)
{
    if (nFold >= k || nFold < 0) {
        throw out_of_range("nFold (" + to_string(nFold) + ") must be less than k (" + to_string(k) + ")");
    }
    vector<int> test_indices = stratified_indices[nFold];
    vector<int> train_indices;
    for (int i = 0; i < k; ++i) {
        if (i == nFold) continue;
        train_indices.insert(train_indices.end(), stratified_indices[i].begin(), stratified_indices[i].end());
    }
    return { train_indices, test_indices };
}