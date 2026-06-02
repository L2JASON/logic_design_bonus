#ifndef SEARCH_H
#define SEARCH_H

// Cyclic Core 탐색 (이신형)
// EPI만으로 못 끝낸 나머지를, 남은 minterm을 최소 비용으로 덮는 PI 조합으로
// 채운다. 일종의 minimum set cover를 Branch & Bound DFS로 푼다.
// 가지치기 조건, MRV 휴리스틱, 비용 공식은 README의 search 항목 참고.

#include "types.h"
#include <vector>

// 비용이 같은 최적해가 여러 개면 다 담아서 반환한다 (출력 방식은 미정).
// 각 후보의 selectedPIs에는 EPI까지 포함된다.
std::vector<SOPCandidate> solveCyclicCore(
    const ChartResult& chart,
    int numVars
);

#endif // SEARCH_H
