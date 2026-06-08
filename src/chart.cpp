// PI Chart 구성 + EPI 선택 + 축소 (이준혁)
// 절차 상세는 README의 chart 항목 / docs/chart.md

#include "chart.h"
#include <algorithm>

using namespace std;

// term이 minterm을 덮나?
static bool covers(const CombinedTerm& t, int mintermValue) {
    return (t.coveredMinterms >> mintermValue) & 1ULL;
}

// 비트마스크 부분집합 sub ⊆ sup?
static bool isSubset(Bits sub, Bits sup) {
    return (sub & sup) == sub;
}

// rows 중 이 minterm을 덮는 행 인덱스 목록
static vector<int> coveringRows(int mintermValue, const vector<PrimeImplicantRow>& rows) {
    vector<int> out;
    for (int r = 0; r < (int)rows.size(); r++)
        if (covers(rows[r].term, mintermValue))
            out.push_back(r);
    return out;
}

// PI -> 행, f=1 minterm -> 열. 각 열에 그 minterm을 덮는 PI 인덱스를 채운다.
static ChartResult buildInitialChart(const vector<CombinedTerm>& pis, const vector<int>& minterms) {
    ChartResult chart;

    for (const auto& pi : pis)
        chart.remainingRows.push_back({ pi, false });

    for (int mv : minterms) {
        MintermColumn col;
        col.mintermValue = mv;
        col.coveredByRows = coveringRows(mv, chart.remainingRows);
        col.covered = false;
        chart.uncoveredCols.push_back(col);
    }
    return chart;
}

// EPI 확정: 어떤 열을 덮는 PI가 딱 하나면 그 PI는 필수. 그 EPI가 덮는 열은 covered 처리.
static bool confirmEssentialPIs(ChartResult& chart) {
    bool changed = false;
    int columnSize = chart.uncoveredCols.size();
    for (int col = 0; col < columnSize; col++) {
        if (chart.uncoveredCols[col].covered) continue;
        if (chart.uncoveredCols[col].coveredByRows.size() != 1) continue;

        PrimeImplicantRow& epiRow = chart.remainingRows[chart.uncoveredCols[col].coveredByRows[0]];
        epiRow.selected = true;
        chart.confirmedEPI.push_back(epiRow.term);
        for (auto& c : chart.uncoveredCols)
            if (!c.covered && covers(epiRow.term, c.mintermValue))
                c.covered = true;
        changed = true;
    }
    return changed;
}

// 열 지배: 열 A를 덮는 PI 집합이 열 B의 것을 포함하면(A⊇B) A(더 쉬운 열)를 지운다.
static bool reduceColumnDominance(ChartResult& chart) {
    bool changed = false;
    int columnSize = chart.uncoveredCols.size();
    for (int a = 0; a < columnSize; a++) {
        if (chart.uncoveredCols[a].covered) continue;
        for (int b = 0; b < columnSize; b++) {
            if (a == b || chart.uncoveredCols[b].covered) continue;
            // B ⊆ A ? (A,B의 coveredByRows는 둘 다 오름차순이라 includes 사용 가능)
            if (includes(chart.uncoveredCols[a].coveredByRows.begin(),
                        chart.uncoveredCols[a].coveredByRows.end(),
                        chart.uncoveredCols[b].coveredByRows.begin(),
                        chart.uncoveredCols[b].coveredByRows.end())) {
                chart.uncoveredCols[a].covered = true;  // A 삭제
                changed = true;
                break;
            }
        }
    }
    return changed;
}

// 행 지배: PI X가 덮는 (남은)열이 PI Y에 포함되고(X⊆Y) Y의 리터럴이 더 적으면 X를 지운다.
// 비용은 popcount(mask) 비교, strict >. 동률이면 둘 다 최적해에 쓰일 수 있어 지우지 않음.
static bool reduceRowDominance(ChartResult& chart) {
    bool changed = false;
    int rowSize = chart.remainingRows.size();

    // 현재 안 덮인 열들의 비트마스크 (남은 열만 비교 대상)
    Bits uncoveredMask = 0;
    for (const auto& c : chart.uncoveredCols)
        if (!c.covered)
            uncoveredMask |= (1ULL << c.mintermValue);

    for (int x = 0; x < rowSize; x++) {
        if (chart.remainingRows[x].selected) continue;
        Bits xCov = chart.remainingRows[x].term.coveredMinterms & uncoveredMask;
        for (int y = 0; y < rowSize; y++) {
            if (x == y || chart.remainingRows[y].selected) continue;
            Bits yCov = chart.remainingRows[y].term.coveredMinterms & uncoveredMask;
            if (isSubset(xCov, yCov) &&
                __builtin_popcountll(chart.remainingRows[y].term.mask)
                    > __builtin_popcountll(chart.remainingRows[x].term.mask)) {
                chart.remainingRows[x].selected = true;  // X 삭제
                changed = true;
                break;
            }
        }
    }
    return changed;
}

// selected 행·covered 열을 빼고, 남은 행 기준으로 coveredByRows를 재계산해 반환.
static ChartResult buildResult(const ChartResult& chart) {
    ChartResult result;
    result.confirmedEPI = chart.confirmedEPI;

    for (const auto& row : chart.remainingRows)
        if (!row.selected)
            result.remainingRows.push_back(row);

    // 행을 솎으면 인덱스가 밀리므로, 새 remainingRows 기준으로 다시 매긴다.
    for (const auto& col : chart.uncoveredCols) {
        if (col.covered) continue;
        MintermColumn newCol;
        newCol.mintermValue = col.mintermValue;
        newCol.covered = false;
        newCol.coveredByRows = coveringRows(col.mintermValue, result.remainingRows);
        result.uncoveredCols.push_back(newCol);
    }
    return result;
}

ChartResult buildAndReduceChart(
    const vector<CombinedTerm>& pis,
    const vector<int>& minterms
) {
    ChartResult chart = buildInitialChart(pis, minterms);

    // EPI 확정 / 열 지배 / 행 지배를 변화가 없을 때까지 반복(고정점).
    // |= 라서 셋 다 매번 실행되고, 하나라도 바꿨으면 한 바퀴 더 돈다.
    bool changed;
    do {
        changed = false;
        changed |= confirmEssentialPIs(chart);
        changed |= reduceColumnDominance(chart);
        changed |= reduceRowDominance(chart);
    } while (changed);

    return buildResult(chart);
}
