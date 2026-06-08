// 입력 파싱 + 결과 출력 (방하영)
// 입력 포맷과 항->문자열 규칙은 README의 format 항목에.
#include "format.h"
#include <iostream>
#include <string>

InputData parseInput(std::istream& in) {
    // TODO(방하영): 변수 수, minterm 수, dc 수, minterm 목록, dc 목록 순으로 읽기.
    (void)in;
    InputData data;
    data.numVars = 0;

    //변수 수를 읽어오는데 입력되지 않을 경우 고려해서 바로 반환
    if (!(in >> data.numVars)) return data;

    //minterm 수 읽어 오기
    int num_minterms;
    in >> num_minterms;

    //Don't care 수 읽어 오기
    int num_dontcares;
    in >> num_dontcares;

    data.minterms.resize(num_minterms); // minterm 저장 공간 할당
    for (int i = 0; i < num_minterms; ++i) {
        in >> data.minterms[i];
    }

    data.dontCares.resize(num_dontcares); //don't care 저장 공간 할당
    for (int i = 0; i < num_dontcares; ++i) {
        in >> data.dontCares[i];
    }

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
