// 입력 파싱 + 결과 출력 (방하영)
// 입력 포맷과 항->문자열 규칙은 README의 format 항목에.

#include "format.h"

InputData parseInput(std::istream& in) {
    // TODO(방하영): 변수 수, minterm 수, dc 수, minterm 목록, dc 목록 순으로 읽기.
    (void)in;
    InputData data;
    data.numVars = 0;
    return data;
}

void printResult(
    const std::vector<SOPCandidate>& candidates,
    int numVars
) {
    // TODO(방하영): 각 후보를 "f(...) = ... + ..." 식과 비용 줄로 출력.
    (void)candidates; (void)numVars;
}

std::string termToString(const CombinedTerm& term, int numVars) {
    // TODO(방하영): 변수 자리마다 mask=1이면 생략, value=1이면 xi, value=0이면 xi'.
    (void)term; (void)numVars;
    return "";
}
