#ifndef CHART_H
#define CHART_H

// PI Chart 구성 + EPI 선택 + 차트 축소 (이준혁)
// 한 열을 덮는 PI가 하나뿐이면 그 PI는 EPI로 확정하고, EPI로 다 못 덮은
// 부분(cyclic core)만 search로 넘긴다. 절차 상세는 README의 chart 항목 참고.

#include "types.h"
#include <vector>

// minterms에는 f=1만 들어와야 한다 (don't care가 섞이면 EPI 판정이 틀어짐).
// 이건 호출하는 main에서 보장한다.
ChartResult buildAndReduceChart(
    const std::vector<CombinedTerm>& pis,
    const std::vector<int>& minterms
);

#endif // CHART_H
