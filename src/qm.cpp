// Prime Implicant 유도 (김태현)
// 병합 공식은 README의 qm 항목에.

#include "qm.h"

std::vector<CombinedTerm> generatePI(
    const std::vector<int>& minterms,
    const std::vector<int>& dontCares,
    int numVars
) {
    // TODO(김태현): 비트 수로 그룹 나누고, 인접 그룹끼리 1비트 차이 항을 반복 병합.
    //              끝까지 안 합쳐진 항을 모아서 반환.
    (void)minterms; (void)dontCares; (void)numVars;
    return {};
}
