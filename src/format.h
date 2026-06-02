#ifndef FORMAT_H
#define FORMAT_H

// 입력 파싱 + 결과 출력 (방하영)
// 입력 5줄 포맷과 항->문자열 규칙, 출력 예시는 README의 format 항목 참고.

#include "types.h"
#include <iostream>
#include <vector>
#include <string>

// cin이든 파일이든 istream으로 받아 한 함수로 처리한다.
InputData parseInput(std::istream& in);

// candidates는 비용이 같은 최적해 묶음. 각 selectedPIs엔 EPI가 들어있다.
void printResult(
    const std::vector<SOPCandidate>& candidates,
    int numVars
);

// 항 하나를 "x1x2'x4" 꼴로. printResult가 쓰지만, 따로 검증하려고 헤더에 노출.
std::string termToString(const CombinedTerm& term, int numVars);

#endif // FORMAT_H
