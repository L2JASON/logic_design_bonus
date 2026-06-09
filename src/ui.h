#ifndef UI_H
#define UI_H

// 대화형 콘솔 메뉴 UI (통합 실행파일용)
// 인자 없이 실행하면 메뉴가 뜨고, 메뉴에서 입력 방식을 골라 파이프라인을 돌린다.
// 파이프라인 자체(PI 유도 -> 차트 축소 -> 탐색 -> 출력)는 runPipeline 하나로 모아,
// 파일 모드(main)와 메뉴 모드가 같은 코드를 쓴다.

#include "types.h"

// 입력 한 건을 받아 최소화 결과까지 출력한다.
void runPipeline(const InputData& input);

// 대화형 메뉴 루프. 프로그램 종료 코드를 반환한다(정상 종료 0).
int runInteractiveUI();

#endif // UI_H
