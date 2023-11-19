#ifndef STREE_H
#define STREE_H
#include "nlohmann/json.hpp"
#include "PyClassifier.h"

namespace pywrap {
    class STree : public PyClassifier {
    public:
        STree();
        ~STree() = default;
        std::string graph();
    };
} /* namespace pywrap */
#endif /* STREE_H */