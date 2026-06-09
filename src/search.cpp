// Cyclic Core 탐색 (이신형)
// 가지치기 조건, MRV, 비용 공식은 README의 search 항목에.

#include "search.h"

void recursive(
    Bits currentCovered, 
    const ChartResult& chart,
    std::vector<CombinedTerm>& currentPIs,
    std::vector<SOPCandidate>& SOPCandidates
){
    int i, j, min = 64, minIndex = -1;


    for(i = 0; i<chart.uncoveredCols.size(); i++){
        int currentMinterm = chart.uncoveredCols[i].mintermValue;

        if (((currentCovered >> currentMinterm) & 1) == 1){
            continue;
        }

        if (chart.uncoveredCols[i].coveredByRows.size() < min){
            min = chart.uncoveredCols[i].coveredByRows.size();
            minIndex = i;
        }
    }

    if (minIndex == -1){
        SOPCandidate candidate;
        candidate.selectedPIs = currentPIs;
        candidate.literalCount = 0;
        candidate.inverterCount = 0;

        SOPCandidates.push_back(candidate);
        return;
    }

    for(j = 0; j<chart.uncoveredCols[minIndex].coveredByRows.size(); j++){
        int piIndex = chart.uncoveredCols[minIndex].coveredByRows[j];

        currentPIs.push_back(chart.remainingRows[piIndex].term);

        Bits piMinterms = chart.remainingRows[piIndex].term.coveredMinterms;
        Bits nextCovered = currentCovered | piMinterms;

        recursive(nextCovered, chart, currentPIs, SOPCandidates);

        currentPIs.pop_back();
    }

    return;
}

std::vector<SOPCandidate> solveCyclicCore(
    const ChartResult& chart,
    int numVars
) {
    // TODO(이신형): 안 덮인 열 중 후보 PI가 가장 적은 열을 골라, 그 열을 덮는
    //              PI들로 분기하며 DFS. 최소 비용 조합(들)을 모아 반환.
    Bits coveredValue = 0;

    std::vector <CombinedTerm> currentPIs;

    std::vector <SOPCandidate> SOPCandidates;

    recursive(coveredValue, chart, currentPIs, SOPCandidates);

    

    for(int k = 0; k<currentPIs.size(); k++){
    int lit = 0;
    int inv = 0;

    SOPCandidates[k].literalCount = lit;
    SOPCandidates[k].inverterCount = inv;
    }

    return SOPCandidates;
}
