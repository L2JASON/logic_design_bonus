#include "types.h"
#include "qm.h"
#include "chart.h"
#include "search.h"
#include "format.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    // 인자로 파일 경로가 오면 파일에서, 없으면 키보드에서 읽는다.
    InputData input;
    if (argc >= 2) {
        std::ifstream fin(argv[1]);
        if (!fin) {
            std::cerr << "Cannot open file: " << argv[1] << "\n";
            return 1;
        }
        input = parseInput(fin);
    } else {
        input = parseInput(std::cin);
    }

    std::vector<CombinedTerm> pis =
        generatePI(input.minterms, input.dontCares, input.numVars);

    // 디버깅용: PI가 손으로 그린 K-map과 맞는지 여기서 찍어 비교.
    // for (const auto& t : pis)
    //     std::cout << termToString(t, input.numVars) << "\n";

    // 차트엔 f=1 minterm만 넘긴다 (don't care는 덮을 대상이 아님).
    ChartResult chart = buildAndReduceChart(pis, input.minterms);

    std::vector<SOPCandidate> candidates =
        solveCyclicCore(chart, input.numVars);

    printResult(candidates, input.numVars);
    return 0;
}
