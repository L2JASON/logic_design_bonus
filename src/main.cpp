// 전체 파이프라인 진입점 (이준혁)
// 인자로 파일 경로가 오면 파일을 읽어 한 번 돌리고, 인자가 없으면 대화형 메뉴(ui)를
// 띄운다. 실제 파이프라인(parseInput -> generatePI -> buildAndReduceChart ->
// solveCyclicCore -> printResult)은 ui.cpp의 runPipeline에 모아 두 모드가 공유한다.
// 각 단계가 주고받는 자료형은 types.h, 알고리즘 설명은 README.

#include "types.h"
#include "format.h"
#include "ui.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    // 인자가 없으면 대화형 콘솔 메뉴로 진입.
    if (argc < 2) {
        return runInteractiveUI();
    }

    // 인자로 파일 경로가 오면 파일에서 읽어 한 번 실행 (기존 동작 유지).
    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }
    InputData input = parseInput(fin);
    runPipeline(input);
    return 0;
}
