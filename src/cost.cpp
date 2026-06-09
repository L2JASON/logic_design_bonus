// 비용 계산 + 최소 비용 선택 (이준혁)
// 비용 정의(product/literal/inverter)와 비교 순서는 README의 search 항목에.

#include "cost.h"

// 항 하나의 리터럴 수 = don't care 자리 빼고 남은 변수 수
int countLiterals(const CombinedTerm& term, int numVars) {
    Bits nMask = makeNBitMask(numVars); // 하위 n비트만 진짜 변수
    int dc = __builtin_popcountll(term.mask & nMask); // mask=1 자리 = don't care 개수
    return numVars - dc; // 빼면 리터럴
}

// SOP 전체 인버터 수. 변수당 1개(공유)라 항마다 못 세고 전부 모아서 센다.
int countInverters(const std::vector<CombinedTerm>& pis, int numVars) {
    Bits nMask = makeNBitMask(numVars);
    Bits invAll = 0; // 보수(')로 쓰인 변수 자리 모음
    for (const CombinedTerm& t : pis)
        invAll |= (~t.value & ~t.mask & nMask); // value=0,mask=0 자리 = 이 항의 x'들
    return __builtin_popcountll(invAll); // OR로 합쳤으니 중복없이 변수 개수
}

// 후보 하나에 비용 3개 채워넣기
void fillCost(SOPCandidate& cand, int numVars) {
    cand.productCount = (int)cand.selectedPIs.size(); // 항 개수가 곧 product
    int lit = 0;
    for (const CombinedTerm& t : cand.selectedPIs)
        lit += countLiterals(t, numVars); // literal은 항마다 합산
    cand.literalCount = lit;
    cand.inverterCount = countInverters(cand.selectedPIs, numVars); // inverter는 전체에서 한 번
}

// 후보들 중 최소 비용만 남기기. product는 DFS가 이미 최소로 넘기니 literal -> inverter만 본다.
std::vector<SOPCandidate> selectMinimumCost(std::vector<SOPCandidate> cands, int numVars) {
    if (cands.empty()) return cands; // 덮을 게 없으면 그냥 반환
    for (SOPCandidate& c : cands) fillCost(c, numVars); // 일단 전원 비용 채움

    // 1) literal 최소만 남기기
    int minL = cands[0].literalCount; // 첫 후보 기준
    for (const SOPCandidate& c : cands)
        if (c.literalCount < minL) minL = c.literalCount; // 더 작으면 갱신 = 최솟값 찾기
    std::vector<SOPCandidate> afterL;
    for (const SOPCandidate& c : cands)
        if (c.literalCount == minL) afterL.push_back(c); // 같은 것만 추림 (==라 동점 다 살림)

    // 2) 남은 것 중 inverter 최소만 남기기 - 똑같은 패턴
    int minI = afterL[0].inverterCount;
    for (const SOPCandidate& c : afterL)
        if (c.inverterCount < minI) minI = c.inverterCount;
    std::vector<SOPCandidate> result;
    for (const SOPCandidate& c : afterL)
        if (c.inverterCount == minI) result.push_back(c);

    return result; // literal·inverter 둘 다 최소 = 모든 최적해
}
