// Cyclic Core 탐색 (이신형)
// 가지치기 조건, MRV, 비용 공식은 README의 search 항목에.

#include "search.h"

void recursive(Bits currentCovered, const ChartResult& chart){
    int i, j, min = 64, minIndex;


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
        return;
    }

    for(j = 0; j<chart.uncoveredCols[minIndex].coveredByRows.size(); j++){
        int piIndex = chart.uncoveredCols[minIndex].coveredByRows[j];

        //수정중...
        recursive(currentCovered, chart);
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

    recursive(coveredValue, chart);
}
