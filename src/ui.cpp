// 대화형 콘솔 메뉴 UI (통합 실행파일용)
// 메뉴 -> 입력 받기 -> runPipeline 으로 결과 출력. 설명은 ui.h 참고.

#include "ui.h"

#include "qm.h"
#include "chart.h"
#include "search.h"
#include "format.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

void runPipeline(const InputData& input) {
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

// "0 1 2 5" 처럼 한 줄에 들어온 정수들을 파싱한다. 빈 줄이면 빈 벡터.
std::vector<int> readIntLine() {
    std::string line;
    std::getline(std::cin, line);
    std::vector<int> out;
    std::istringstream iss(line);
    int v;
    while (iss >> v) out.push_back(v);
    return out;
}

// n, minterm 목록, don't care 목록을 직접 받아 검증한다.
// 잘못된 입력이면 false를 돌려주고 메뉴로 돌아간다.
bool readDirectInput(InputData& input) {
    std::cout << "변수 개수 n (1~20): ";
    int n;
    if (!(std::cin >> n)) { clearLine(); std::cout << "숫자를 입력해 주세요.\n"; return false; }
    clearLine();  // n 뒤의 개행 제거 (다음 getline 대비)
    if (n < 1 || n > 20) { std::cout << "n은 1~20 범위여야 합니다.\n"; return false; }

    const long long maxMinterm = (1LL << n) - 1;  // 0 ~ 2^n-1
    std::cout << "f=1 minterm (공백 구분, 예: 0 1 2 5): ";
    std::vector<int> minterms = readIntLine();
    std::cout << "don't care (없으면 그냥 Enter): ";
    std::vector<int> dontCares = readIntLine();

    if (minterms.empty()) { std::cout << "f=1 minterm이 하나도 없습니다.\n"; return false; }

    // 범위 검사: 0 <= 값 <= 2^n-1
    auto inRange = [&](int v) { return v >= 0 && v <= maxMinterm; };
    for (int m : minterms)
        if (!inRange(m)) { std::cout << "minterm " << m << " 이(가) 0~" << maxMinterm << " 범위를 벗어났습니다.\n"; return false; }
    for (int d : dontCares)
        if (!inRange(d)) { std::cout << "don't care " << d << " 이(가) 0~" << maxMinterm << " 범위를 벗어났습니다.\n"; return false; }

    input.numVars   = n;
    input.minterms  = minterms;
    input.dontCares = dontCares;
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
