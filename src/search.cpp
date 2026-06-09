// Cyclic Core 탐색 (이신형)
// EPI로 못 끝낸 나머지 민텀을, 최소 비용 PI 조합으로 마저 덮는다.
// 흐름: solveCyclicCore(입구) -> recursive(DFS로 답 모으기) -> 최소비용만 남겨 반환.
// 가지치기 조건, MRV, 비용 공식은 README의 search 항목에.

#include "search.h"
#include "cost.h"      // 비용 계산 + 최소비용 선택 (이준혁 cost 모듈)
#include "algorithm"

// 안 덮인 민텀들을 PI로 마저 덮는 DFS. 한 번 호출 = 트리에서 한 칸 내려가기.
//   currentCovered: 지금까지 덮은 민텀(비트로 표시) / currentPIs: 지금까지 고른 PI들
void recursive(
    Bits currentCovered,
    const ChartResult& chart,
    std::vector<CombinedTerm>& currentPIs,
    std::vector<SOPCandidate>& SOPCandidates,
    int& bestProduct, //: 가지치기용 "지금까지 찾은 최소 product 개수"
    std::vector<bool> excluded //: 중복막이용 "금지된 PI들". 
    //                         -> {0,2}와 {2,0}처럼 순서만 다른 중복 가지를 애초에 안 만들기 위함
){
    // 가지치기: 이미 고른 PI가 best보다 많으면 더 파봐야 손해 -> 여기서 멈춤.
    //   ('>=' 말고 '>'. 비용 같은 답도 살려야 하니까)
    if ((int)currentPIs.size() > bestProduct) return;

    int i, j, min = 999999, minIndex = -1;  // min=가장 적은 PI 개수, minIndex=그 열 번호
    
    for(i = 0; i<chart.uncoveredCols.size(); i++){
        int currentMinterm = chart.uncoveredCols[i].mintermValue;

        if (((currentCovered >> currentMinterm) & 1) == 1ULL){  // 이미 덮은 민텀이면 패스
            continue;
        }

        if (chart.uncoveredCols[i].coveredByRows.size() < min){  // 덮는 PI가 제일 적은 열을 고른다
            min = chart.uncoveredCols[i].coveredByRows.size();
            minIndex = i;
        }
    }

    if (minIndex == -1){  // 고를 열이 없다 = 다 덮었다
        SOPCandidate candidate;
        candidate.selectedPIs = currentPIs;  // 아직 탐색으로 고른 PI만 담음. 앞에 EPI(chart.confirmedEPI)도 붙여야 진짜 답


        // 비용은 일단 0. README 공식으로 채울 자리
        candidate.literalCount = 0;
        candidate.inverterCount = 0;

        SOPCandidates.push_back(candidate);  // 아래 exclude 분기 덕에 같은 집합은 애초에 한 번만 만들어짐 -> 여기선 그냥 넣으면 됨(잎에서 중복검사 불필요)
        if((int)currentPIs.size()<bestProduct){
            bestProduct = (int)currentPIs.size();
        }
        // 그리고 이번 product가 더 작으면 bestProduct도 갱신 (위 가지치기가 쓸 기준값)
        return;
    }

    std::vector<int> sortedPIs = chart.uncoveredCols[minIndex].coveredByRows;

    std::sort(sortedPIs.begin(), sortedPIs.end(), [&](int a, int b){
        return __builtin_popcountll(chart.remainingRows[a].term.coveredMinterms & ~currentCovered) > __builtin_popcountll(chart.remainingRows[b].term.coveredMinterms & ~currentCovered);
    });
    
    // [exclude 분기] 이 열에서 "앞서 이미 시도한 PI"를 모아둘 로컬 금지목록을 따로 둔다.
    //   처음엔 위에서 받은 excluded 그대로 복사해서 시작:  Bits excludedHere = excluded;
    //   원리: "이 열을 PI0로 덮는 경우 / (PI0 빼고) PI1으로 덮는 경우 / (PI0,PI1 빼고) PI2로..." 로 쪼개면
    //         답마다 '제일 먼저 고른 PI'가 딱 하나로 정해져서, 같은 집합이 두 번 안 나온다.
    std::vector<bool> excludedHere = excluded;
    for(j = 0; j<sortedPIs.size(); j++){  // 고른 열을 덮는 PI를 하나씩 시도(분기)

        int piIndex = sortedPIs[j];

        // 이미 금지된 PI면 이번 가지는 건너뛴다. (이게 {2,0} 같은 중복 가지를 통째로 막는 핵심)
        if (excludedHere[piIndex]) continue;

        currentPIs.push_back(chart.remainingRows[piIndex].term);  // 이 PI를 답 후보에 넣고

        Bits piMinterms = chart.remainingRows[piIndex].term.coveredMinterms;
        Bits nextCovered = currentCovered | piMinterms;  // 이 PI가 덮는 민텀까지 합친 상태로

        // 내려갈 땐 excludedHere를 같이 넘긴다 (이 열에서 앞서 시도한 PI들이 담겨 있음).
        recursive(nextCovered, chart, currentPIs, SOPCandidates, bestProduct, excludedHere);

        currentPIs.pop_back();  // 돌아오면 도로 빼기 (다음 PI 시도하려고) = 백트래킹
        excludedHere[piIndex] = true;
        // 다음 형제 가지부터는 방금 PI를 금지로 추가: excludedHere[piIndex] = true;
        //   -> "PI0를 이미 써본 가지에선 PI0를 다시 첫 선택으로 쓰지 않는다" = 순서 중복 제거
    }

    return;
}

std::vector<SOPCandidate> solveCyclicCore(
    const ChartResult& chart,
    int numVars
) {
    // 여기가 입구. 아무것도 안 덮은 빈 상태에서 시작해, recursive가 답들을 SOPCandidates에 모아준다.
    Bits coveredValue = 0;

    std::vector <CombinedTerm> currentPIs;

    std::vector <SOPCandidate> SOPCandidates;

    // 시작값들 만들어 같이 넘기기:
    int bestProduct = 999999;   // 가지치기 기준값 (최대치로 변경)
    std::vector<bool> excluded(chart.remainingRows.size(), false); // 처음엔 금지된 PI 없음
    recursive(coveredValue, chart, currentPIs, SOPCandidates, bestProduct, excluded);
    
    // EPI를 각 후보 앞에 붙인다 (cyclic core로 고른 PI + 필수항 EPI = 진짜 최종 답).
    for (SOPCandidate& c : SOPCandidates)
        c.selectedPIs.insert(c.selectedPIs.begin(),
                             chart.confirmedEPI.begin(), chart.confirmedEPI.end());

    // 비용 채우고 최소만 남겨 반환 (product 최소는 DFS clear가 보장, 여기선 literal -> inverter).
    return selectMinimumCost(SOPCandidates, numVars);
}