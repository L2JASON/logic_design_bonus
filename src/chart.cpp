// PI Chart 구성 + EPI 선택 + 축소 (이준혁)
// 절차 상세는 README의 chart 항목에.

#include "chart.h"
#include <algorithm>

ChartResult buildAndReduceChart(
    const std::vector<CombinedTerm>& pis,
    const std::vector<int>& minterms
) {
    // TODO(이준혁): 행(PI)/열(minterm) 만들고, 한 열을 덮는 PI가 하나뿐이면
    //              EPI 확정. EPI가 덮은 열 지우고 남은 행/열을 반환.

    /*struct CombinedTerm {
        Bits value;            // mask가 0인 자리의 0/1 값 
        Bits mask;             // 1인 자리는 don't care('-')
        Bits coveredMinterms;  // 이 항이 덮는 f=1 minterm들 (don't care 번호는 빼고)
        bool used;             // 병합에 쓰였는지. 끝까지 false인 항이 PI
    };*/

    /*struct ChartResult {
        std::vector<CombinedTerm>      confirmedEPI;   // 먼저 확정된 EPI
        std::vector<PrimeImplicantRow> remainingRows;  // EPI로 못 끝낸 나머지 PI
        std::vector<MintermColumn>     uncoveredCols;  // 아직 안 덮인 minterm
    };*/

    /*struct PrimeImplicantRow {
        CombinedTerm term;
        bool selected;
    };

    struct MintermColumn {
        int mintermValue;               // 덮어야 할 minterm 번호
        std::vector<int> coveredByRows; // 이 minterm을 덮는 PI들의 인덱스.
                                        // 딱 하나면 그 PI는 EPI 확정
        bool covered;
    };*/
    // pi\minterm 표 구조체 선언
    ChartResult chart;

    // 파라미터로 표 구성
    // 1. 파라미터 PIs 배열 -> 행 데이터 (PrimeImplicantRow)
    int rowSize = pis.size();
    for(int i=0;i<rowSize;i++){
        PrimeImplicantRow newPiRow = { pis[i], false };
        chart.remainingRows.push_back(newPiRow);
    }
    // 2. 파라미터 minterms 배열 -> 열 데이터 (MintermColumn)
    int columnSize = minterms.size();
    for(int col=0; col<columnSize; col++){
        MintermColumn newMintermCol;
        newMintermCol.mintermValue = minterms[col];
        for(int row=0;row<rowSize;row++){ // 열 데이터에 민텀을 커버하는 PI를 채우기 위해 행 (PI 인덱스) 순회
            Bits cov = chart.remainingRows[row].term.coveredMinterms; // 행 데이터의 coveredminterm 복사
            if ((cov >> newMintermCol.mintermValue) & 1ULL) { // minterms 자릿값까지 쭉 시프트 그 비트가 1 인가?
                newMintermCol.coveredByRows.push_back(row);
            }
        }
        newMintermCol.covered = false;
        chart.uncoveredCols.push_back(newMintermCol);
    }
    // 소거 알고리즘
    // EPI 찾기
    bool foundNew;
    do {
        foundNew = false;
        for (int col = 0; col < columnSize; col++) {
            if (chart.uncoveredCols[col].covered) continue;
            // 1. EPI 확정
            // 아직 안 뽑힌 PI 중 이 열을 덮는 게 몇 개인지 센다 딱 하나면 그 PI가 EPI
            if (chart.uncoveredCols[col].coveredByRows.size() == 1) {
                PrimeImplicantRow& epiRow = chart.remainingRows[chart.uncoveredCols[col].coveredByRows[0]];
                // 그 PI를 EPI 확정: selected=true
                epiRow.selected = true;
                // confirmedEPI에 추가
                chart.confirmedEPI.push_back(epiRow.term);
                // 그 PI가 덮는 열들 covered=true
                Bits cov = epiRow.term.coveredMinterms; // epi의 coveredMinterms Bit 복사
                for (int i = 0; i < columnSize; i++){
                    // coverd=false 인 열을 순회하며 해당 열 민텀 자릿값까지 쭉 시프트, 그 비트가 1 인가?
                    if (!chart.uncoveredCols[i].covered && (cov >> chart.uncoveredCols[i].mintermValue) & 1ULL) {
                        chart.uncoveredCols[i].covered = true;
                    }
                }
                foundNew = true;
            }
        }
        // 2. 열 지배 축소
        for (int a = 0; a<columnSize; a++){
            if(chart.uncoveredCols[a].covered) continue;
            for(int b = 0; b<columnSize; b++){
                if(a==b||chart.uncoveredCols[b].covered) continue;
                bool isInclude = std::includes(
                    chart.uncoveredCols[a].coveredByRows.begin(),
                    chart.uncoveredCols[a].coveredByRows.end(),
                    chart.uncoveredCols[b].coveredByRows.begin(),
                    chart.uncoveredCols[b].coveredByRows.end());
                if (isInclude) {
                    chart.uncoveredCols[a].covered = true;
                    foundNew = true;
                    break;
                }
            }
        }
        // 3. 행 지배 축소
        // 현재 안 덮인 열들의 비트마스크 생성
        Bits uncoveredMask = 0;
        for (int c = 0; c < columnSize; c++){
            if (!chart.uncoveredCols[c].covered){
                uncoveredMask |= (1ULL << chart.uncoveredCols[c].mintermValue);
            }
        }
        for (int x = 0; x<rowSize; x++){
            if(chart.remainingRows[x].selected) continue;
            Bits xCov = chart.remainingRows[x].term.coveredMinterms & uncoveredMask;
            for(int y = 0; y<rowSize; y++){
                if(x==y||chart.remainingRows[y].selected) continue;
                Bits yCov = chart.remainingRows[y].term.coveredMinterms & uncoveredMask;
                if ((xCov & yCov) == xCov) { // x ⊆ y?
                    // lteral 수 비용을 따졌을때 x를 소거해도 되는가? x가 더 저렴하다면 소거 금지
                    if (__builtin_popcountll(chart.remainingRows[y].term.mask) > __builtin_popcountll(chart.remainingRows[x].term.mask)) {
                        // x 소거
                        chart.remainingRows[x].selected = true;
                        foundNew = true;
                        break;
                    }
                }
            }
        }
    } while (foundNew);

    // 반환할 결과 차트 생성
    ChartResult result;
    result.confirmedEPI = chart.confirmedEPI;
    // selected == flase 행 만 추가
    for(int i=0; i<rowSize; i++){
        if (!chart.remainingRows[i].selected){
            result.remainingRows.push_back(chart.remainingRows[i]);
        }
    }

    // covered == false 열 만 추가
    int resultRowSize = result.remainingRows.size();
    // result 에서 변화한 행인덱스에 맞게 coveredByRows 재계산
    for(int i=0; i<columnSize; i++){
        if (!chart.uncoveredCols[i].covered){
            MintermColumn newUncoveredCol;
            newUncoveredCol.mintermValue = chart.uncoveredCols[i].mintermValue;
            newUncoveredCol.covered = false;
            for(int row=0;row<resultRowSize;row++){ // 열 데이터에 민텀을 커버하는 PI를 채우기 위해 행 (PI 인덱스) 순회
                Bits cov = result.remainingRows[row].term.coveredMinterms; // 행 데이터의 coveredminterm 복사
                if ((cov >> newUncoveredCol.mintermValue) & 1ULL) { // minterms 자릿값까지 쭉 시프트 그 비트가 1 인가?
                    newUncoveredCol.coveredByRows.push_back(row);
                }
            }
            result.uncoveredCols.push_back(newUncoveredCol);
        }
    }
    
    return result;
}
