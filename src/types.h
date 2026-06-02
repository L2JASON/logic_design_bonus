#ifndef TYPES_H
#define TYPES_H

// 모든 모듈이 공유하는 자료형. 여기 필드를 바꾸면 4명 코드가 다 영향받으니
// 변경은 팀 합의 후에. 설계 근거와 비트 연산 공식은 README에 정리해둠.

#include <cstdint>
#include <vector>
#include <string>

using Bits = uint64_t;   // 비트 연산용. 시프트할 때 1ULL 써야 함 (1 << 63은 UB)

// 헷갈리는 부분: 아래 구조체에 비트필드가 두 종류 있는데 좌표계가 다르다.
//   value, mask     -> 비트 위치 = 변수 번호 (하위 n비트만 씀)
//   coveredMinterms -> 비트 위치 = minterm 번호 (0 ~ 2^n-1)
// 둘을 섞어 쓰면 무조건 버그. 예: n=4에서 항 x1 x2' - x4 는
//   value=1001, mask=0010 (x3 자리가 don't care)

struct CombinedTerm {
    Bits value;            // mask가 0인 자리의 0/1 값
    Bits mask;             // 1인 자리는 don't care('-')
    Bits coveredMinterms;  // 이 항이 덮는 f=1 minterm들 (don't care 번호는 빼고)
    bool used;             // 병합에 쓰였는지. 끝까지 false인 항이 PI
};

struct PrimeImplicantRow {
    CombinedTerm term;
    Bits coveredMinterms;  // term.coveredMinterms 사본. 차트에서 자주 비교해서 따로 둠
    bool selected;
};

struct MintermColumn {
    int mintermValue;               // 덮어야 할 minterm 번호
    std::vector<int> coveredByRows; // 이 minterm을 덮는 PI들의 인덱스.
                                    // 딱 하나면 그 PI는 EPI 확정
    bool covered;
};

struct SOPCandidate {
    std::vector<CombinedTerm> selectedPIs;  // EPI + 탐색으로 고른 PI 전부
    int productCount;   // 곱항 수
    int literalCount;   // 리터럴 총개수
    int inverterCount;  // 보수(') 붙은 리터럴 수
};

struct ChartResult {
    std::vector<CombinedTerm>      confirmedEPI;   // 먼저 확정된 EPI
    std::vector<PrimeImplicantRow> remainingRows;  // EPI로 못 끝낸 나머지 PI
    std::vector<MintermColumn>     uncoveredCols;  // 아직 안 덮인 minterm
};

struct InputData {
    int numVars;                 // 변수 개수 n
    std::vector<int> minterms;   // f=1 minterm
    std::vector<int> dontCares;
};

// 하위 n비트만 1인 마스크. uint64_t에서 우리가 실제로 쓰는 범위를 자를 때 사용.
inline Bits makeNBitMask(int n) {
    return (n >= 64) ? ~0ULL : ((1ULL << n) - 1);   // n==64면 1ULL<<64가 UB라 분기
}

#endif // TYPES_H
