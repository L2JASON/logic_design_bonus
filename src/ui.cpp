// 대화형 콘솔 메뉴 UI (통합 실행파일용)
// 메뉴 -> 입력 받기 -> runPipeline 으로 결과 출력. 설명은 ui.h 참고.

#include "ui.h"

#include "qm.h"
#include "chart.h"
#include "search.h"
#include "format.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

// 입력값이 파이프라인에서 안전한지 공통 검증 (직접 입력/파일/CLI 모두 여기서 걸린다).
// 형식 파싱은 format(parseInput)이 맡고, UI는 안내만 한다 — 값의 유효 범위는 여기서 본다.
//  - n은 1~6: coveredMinterms가 64비트라 minterm 번호 0~63까지만 비트로 담긴다
//             (qm.cpp의 1ULL << mintermNumber). 벗어나면 표현 불가/UB.
//  - 모든 minterm/dontcare 번호는 0 ~ 2^n-1.
static bool validateInput(const InputData& input) {
    if (input.numVars < 1 || input.numVars > 6) {
        std::cout << "변수 개수 n은 1~6이어야 합니다 (minterm 0~63만 표현 가능).\n";
        return false;
    }
    if (input.minterms.empty()) {
        std::cout << "f=1 minterm이 하나도 없습니다.\n";
        return false;
    }
    const long long maxMinterm = (1LL << input.numVars) - 1;  // 0 ~ 2^n-1
    for (int m : input.minterms)
        if (m < 0 || m > maxMinterm) { std::cout << "minterm " << m << " 이(가) 0~" << maxMinterm << " 범위를 벗어났습니다.\n"; return false; }
    for (int d : input.dontCares)
        if (d < 0 || d > maxMinterm) { std::cout << "don't care " << d << " 이(가) 0~" << maxMinterm << " 범위를 벗어났습니다.\n"; return false; }
    return true;
}

void runPipeline(const InputData& input) {
    if (!validateInput(input)) return;

    std::vector<CombinedTerm> pis =
        generatePI(input.minterms, input.dontCares, input.numVars);

    // 차트엔 f=1 minterm만 넘긴다 (don't care는 덮을 대상이 아님).
    ChartResult chart = buildAndReduceChart(pis, input.minterms);

    std::vector<SOPCandidate> candidates =
        solveCyclicCore(chart, input.numVars);

    if (candidates.empty()) {
        std::cout << "\n최소화 결과가 없습니다. (입력을 확인해 주세요)\n";
        return;
    }
    std::cout << "\n";
    printResult(candidates, input.numVars);
}

namespace {

// cin이 실패 상태면 풀어주고 남은 줄을 버린다(잘못된 입력 후 복구용).
void clearLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 직접 입력: 형식만 안내하고, 실제 파싱은 format의 parseInput에 그대로 맡긴다.
// 파일 모드와 같은 함수를 쓰므로 입력 형식이 100% 일치한다. 값의 유효성(n 범위,
// 번호 범위)은 runPipeline의 validateInput이 공통으로 검사한다 — UI는 안내까지만.
bool readDirectInput(InputData& input) {
    std::cout <<
        "입력 형식 — 아래 순서대로 숫자를 공백/줄바꿈으로 구분해 입력하세요:\n"
        "  1) 변수 개수 n (1~6)\n"
        "  2) f=1 minterm 개수\n"
        "  3) don't care 개수\n"
        "  4) f=1 minterm 목록\n"
        "  5) don't care 목록\n"
        "  예)  4  4  1   0 1 2 5   9\n"
        "입력: ";
    input = parseInput(std::cin);
    clearLine();  // parseInput이 남긴 줄을 정리 (다음 메뉴 입력 대비)
    return true;
}

// 파일 경로를 받아 parseInput으로 읽는다.
bool readFromFile(InputData& input) {
    std::cout << "파일 경로: ";
    std::string path;
    std::getline(std::cin, path);
    if (path.empty()) { std::cout << "경로가 비어 있습니다.\n"; return false; }

    std::ifstream fin(path);
    if (!fin) { std::cout << "파일을 열 수 없습니다: " << path << "\n"; return false; }
    input = parseInput(fin);
    if (input.numVars < 1) { std::cout << "파일 형식이 올바르지 않습니다.\n"; return false; }
    return true;
}

void printMenu() {
    std::cout << "\n===== QM Minimizer (5조) =====\n"
            << "1) 직접 입력\n"
            << "2) 파일에서 불러오기\n"
            << "3) 예제 실행 (4변수)\n"
            << "4) 종료\n"
            << "> ";
}

} // namespace

int runInteractiveUI() {
    std::cout << "Quine-McCluskey 최소 SOP 계산기\n";

    while (true) {
        printMenu();

        int choice;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) { std::cout << "\n종료합니다.\n"; return 0; }
            clearLine();
            std::cout << "1~4 중에서 선택해 주세요.\n";
            continue;
        }
        clearLine();  // 선택 숫자 뒤 개행 제거

        InputData input;
        switch (choice) {
            case 1:
                if (readDirectInput(input)) runPipeline(input);
                break;
            case 2:
                if (readFromFile(input)) runPipeline(input);
                break;
            case 3: {
                // 예제: f(x1..x4) = Σm(0,1,2,5) + d(9)
                input.numVars   = 4;
                input.minterms  = {0, 1, 2, 5};
                input.dontCares = {9};
                std::cout << "예제: n=4, minterm={0,1,2,5}, dontcare={9}\n";
                runPipeline(input);
                break;
            }
            case 4:
                std::cout << "종료합니다.\n";
                return 0;
            default:
                std::cout << "1~4 중에서 선택해 주세요.\n";
                break;
        }
    }
}
