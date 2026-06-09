// 입력 파싱 + 결과 출력 (방하영)
// 입력 포맷과 항->문자열 규칙은 README의 format 항목에.
#include "format.h"
#include <iostream>
#include <string>

InputData parseInput(std::istream& in) {
    // TODO(방하영): 변수 수, minterm 수, dc 수, minterm 목록, dc 목록 순으로 읽기.
    InputData data;
    data.numVars = 0;

    // 변수 수를 읽어오는데 입력되지 않을 경우 고려해서 바로 반환
    if (!(in >> data.numVars)) return data;

    // minterm 수 읽어 오기
    int num_minterms;
    in >> num_minterms;

    // Don't care 수 읽어 오기
    int num_dontcares;
    in >> num_dontcares;

    data.minterms.resize(num_minterms); // minterm 저장 공간 할당
    for (int i = 0; i < num_minterms; ++i) {
        in >> data.minterms[i];
    }

    data.dontCares.resize(num_dontcares); // don't care 저장 공간 할당
    for (int i = 0; i < num_dontcares; ++i) {
        in >> data.dontCares[i];
    }

    return data;
}

std::string termToString(const CombinedTerm& term, int numVars) {
    // TODO(방하영): 변수 자리마다 mask=1이면 생략, value=1이면 xi, value=0이면 xi'.
    std::string result = "";
    // 변수 수만큼 자리 검사
    for (int i = 0; i < numVars; ++i) {
        // 왼쪽부터 차례대로 읽어서 1인 부분 찾기
        int bit_pos = numVars - i - 1;

        //mask=1이면 생략 함수 선언
        bool is_dont_care = (term.mask & (1ULL << bit_pos)) != 0; 

        if (!is_dont_care) {
            //value=1이면 xi, value=0이면 xi'
            bool bit_val = (term.value & (1ULL << bit_pos)) != 0;

            result += "x" + std::to_string(i + 1);

            if (!bit_val) { //비트 값이 0이면 기호추가
                result += "'";
            }

        }
    }
    if (result.empty()) {
        return "1";
    }
    return result;
        
    }




void printResult(const std::vector<SOPCandidate>& candidates, int numVars) {
    // TODO(방하영): 각 후보를 "f(...) = ... + ..." 식과 비용 줄로 출력.

    if (candidates.empty()) return; // 최적화가 안되는 식이면 바로 출력

    for (const auto& candidate : candidates) { // 간소화 한 식이 여러개일 수 있음
        std::cout << "Minimized SOP:\n";

        // 함수 출력
        std::cout << "f(";
        for (int i = 1; i <= numVars; ++i) { 
            std::cout << "x" << i << (i == numVars ? "" : ", ");
        }
        std::cout << ") = ";

        // 간소화 식 출력
        for (size_t i = 0; i < candidate.selectedPIs.size(); ++i) {
            std::cout << termToString(candidate.selectedPIs[i], numVars);

            // 마지막 항 제외 + 넣기
            if (i != candidate.selectedPIs.size() - 1) {
                std::cout << " + ";
            }
        }
        std::cout << "\n";
        // 마지막 Cost 출력
        std::cout << "Cost: product count = " << candidate.productCount << ", literal count = " << candidate.literalCount << "\n\n";
    }
}
