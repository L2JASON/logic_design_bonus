// Cyclic Core 탐색 (이신형)
// 가지치기 조건, MRV, 비용 공식은 README의 search 항목에.

#include "search.h"

std::vector<SOPCandidate> solveCyclicCore(
    const ChartResult& chart,
    int numVars
) {
    // TODO(이신형): 안 덮인 열 중 후보 PI가 가장 적은 열을 골라, 그 열을 덮는
    //              PI들로 분기하며 DFS. 최소 비용 조합(들)을 모아 반환.
    (void)chart; (void)numVars;
    return {};
}
