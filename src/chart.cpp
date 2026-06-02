// PI Chart 구성 + EPI 선택 + 축소 (이준혁)
// 절차 상세는 README의 chart 항목에.

#include "chart.h"

ChartResult buildAndReduceChart(
    const std::vector<CombinedTerm>& pis,
    const std::vector<int>& minterms
) {
    // TODO(이준혁): 행(PI)/열(minterm) 만들고, 한 열을 덮는 PI가 하나뿐이면
    //              EPI 확정. EPI가 덮은 열 지우고 남은 행/열을 반환.
    (void)pis; (void)minterms;
    ChartResult result;
    return result;
}
