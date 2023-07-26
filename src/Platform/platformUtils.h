#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H
#include <torch/torch.h>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include "ArffFiles.h"
#include "CPPFImdlp.h"
using namespace std;
const string PATH = "../../data/";

bool file_exists(const std::string& name);
pair<vector<mdlp::labels_t>, map<string, int>> discretize(vector<mdlp::samples_t>& X, mdlp::labels_t& y, vector<string> features);
vector<mdlp::labels_t> discretizeDataset(vector<mdlp::samples_t>& X, mdlp::labels_t& y);
pair<torch::Tensor, map<string, vector<int>>> discretizeTorch(torch::Tensor& X, torch::Tensor& y, vector<string>& features, string className);
tuple<vector<vector<int>>, vector<int>, vector<string>, string, map<string, vector<int>>> loadFile(string name);
tuple<torch::Tensor, torch::Tensor, vector<string>, string, map<string, vector<int>>> loadDataset(string path, string name, bool class_last, bool discretize_dataset);
map<string, vector<int>> get_states(vector<string>& features, string className, map<string, int>& maxes);
#endif //PLATFORM_UTILS_H
