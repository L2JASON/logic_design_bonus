#ifndef QM_H
#define QM_H

// Prime Implicant 유도 (김태현)
// minterm과 don't care로부터 더 못 합치는 항(PI)을 전부 뽑는다.
// 병합 판정/결과 공식은 README의 qm 항목 참고.

#include "types.h"
#include <vector>

// don't care는 병합에는 쓰지만, 반환되는 항의 coveredMinterms에는 넣지 않는다
// (안 그러면 차트에 don't care 열이 생긴다).
std::vector<CombinedTerm> generatePI(
    const std::vector<int>& minterms,
    const std::vector<int>& dontCares,
    int numVars
);

#endif // QM_H
