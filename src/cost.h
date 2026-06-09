#ifndef COST_H
#define COST_H

#include "types.h"
#include <vector>

int countLiterals(const CombinedTerm& term, int numVars);
int countInverters(const std::vector<CombinedTerm>& pis, int numVars);
void fillCost(SOPCandidate& cand, int numVars);

std::vector<SOPCandidate> selectMinimumCost(std::vector<SOPCandidate> cands, int numVars);

#endif // COST_H