// PI Chart 구성 + EPI 선택 + 축소 (이준혁)
// 절차 상세는 README의 chart 항목에.

#include "chart.h"

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
    // 결과 구조체 선언
    ChartResult result;

    // 파라미터로 표 구성
    // 1. 파라미터 PIs 배열 -> 행 데이터 (PrimeImplicantRow)
    int rowSize = pis.size();
    for(int i=0;i<rowSize;i++){
        PrimeImplicantRow newPiRow = { pis[i], false };
        result.remainingRows.push_back(newPiRow);
    }
    // 2. 파라미터 minterms 배열 -> 열 데이터 (MintermColumn)
    int columnSize = minterms.size();
    for(int col=0; col<columnSize; col++){
        MintermColumn newMintermCol;
        newMintermCol.mintermValue = minterms[col];
        for(int row=0;row<rowSize;row++){ // 열 데이터에 민텀을 커버하는 PI를 채우기 위해 행 (PI 인덱스) 순회
            Bits cov = result.remainingRows[row].term.coveredMinterms; // 행 데이터의 coveredminterm 복사
            if ((cov >> newMintermCol.mintermValue) & 1ULL) { // minterms 자릿값까지 쭉 시프트 그 비트가 1 인가?
                newMintermCol.coveredByRows.push_back(row);
            }
        }
        newMintermCol.covered = false;
        result.uncoveredCols.push_back(newMintermCol);
    }
    // 소거 알고리즘
    return result;
}
