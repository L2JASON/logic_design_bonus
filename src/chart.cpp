// PI Chart 구성 + EPI 선택 + 축소 (이준혁)
// 흐름: 차트 만들기 -> 필수 PI(EPI) 뽑기 -> 뻔한 행·열 쳐내기, 를 더 줄 게 없을 때까지 반복.
//   다 줄이고 남은 부분(cyclic core)만 search로 넘긴다.
// 절차 상세는 README의 chart 항목 / docs/chart.md

#include "chart.h"
#include <algorithm>

using namespace std;

// 이 term이 그 minterm을 덮고 있나? (coveredMinterms의 해당 비트 한 칸 보면 됨)
static bool covers(const CombinedTerm& t, int mintermValue) {
    return (t.coveredMinterms >> mintermValue) & 1ULL;
}

// sub가 sup 안에 통째로 들어가나? (부분집합) sub의 1들이 전부 sup에도 1이면 OK
static bool isSubset(Bits sub, Bits sup) {
    return (sub & sup) == sub;
}

// 이 minterm을 덮는 행이 누구누구인지, 행 인덱스로 모아준다
static vector<int> coveringRows(int mintermValue, const vector<PrimeImplicantRow>& rows) {
    vector<int> out;
    for (int r = 0; r < (int)rows.size(); r++)
        if (covers(rows[r].term, mintermValue))
            out.push_back(r);
    return out;
}

// 차트 만들기: PI는 행으로, f=1 minterm은 열로. 그리고 각 열마다 그걸 덮는 PI들을 적어둔다.
static ChartResult buildInitialChart(const vector<CombinedTerm>& pis, const vector<int>& minterms) {
    ChartResult chart;

    for (const auto& pi : pis)
        chart.remainingRows.push_back({ pi, false });  // 모든 PI를 행으로 깔기 (아직 아무도 안 골림)

    for (int mv : minterms) {
        MintermColumn col;
        col.mintermValue = mv;
        col.coveredByRows = coveringRows(mv, chart.remainingRows);  // 이 열을 덮는 PI가 누군지 미리 적어둠
        col.covered = false;
        chart.uncoveredCols.push_back(col);
    }
    return chart;
}

// EPI 뽑기: 어떤 열을 덮는 PI가 딱 하나뿐이면, 걔 없이는 그 민텀을 못 덮으니 무조건 써야 한다 = 필수(EPI).
//   그 EPI가 덮는 열들은 이제 끝난 거니까 covered로 표시해 둔다.
static bool confirmEssentialPIs(ChartResult& chart) {
    bool changed = false;
    int columnSize = chart.uncoveredCols.size();
    for (int col = 0; col < columnSize; col++) {
        if (chart.uncoveredCols[col].covered) continue;          // 이미 끝난 열은 패스
        if (chart.uncoveredCols[col].coveredByRows.size() != 1) continue;  // 덮는 PI가 딱 1개일 때만 EPI

        PrimeImplicantRow& epiRow = chart.remainingRows[chart.uncoveredCols[col].coveredByRows[0]];
        epiRow.selected = true;                  // 이 행은 답에 확정 (더 안 건드림)
        chart.confirmedEPI.push_back(epiRow.term);
        for (auto& c : chart.uncoveredCols)      // 이 EPI가 덮는 열은 전부 끝 처리
            if (!c.covered && covers(epiRow.term, c.mintermValue))
                c.covered = true;
        changed = true;
    }
    return changed;
}

// 열 지배 줄이기: A를 덮는 PI들이 B를 덮는 PI들을 통째로 포함하면(A⊇B),
//   B만 챙기면 A는 어차피 같이 덮이니까 더 쉬운 A를 지워도 된다.
static bool reduceColumnDominance(ChartResult& chart) {
    bool changed = false;
    int columnSize = chart.uncoveredCols.size();
    for (int a = 0; a < columnSize; a++) {
        if (chart.uncoveredCols[a].covered) continue;
        for (int b = 0; b < columnSize; b++) {
            if (a == b || chart.uncoveredCols[b].covered) continue;
            // B의 PI들이 A의 PI들 안에 다 들어가나? (둘 다 오름차순이라 includes로 바로 확인 가능)
            if (includes(chart.uncoveredCols[a].coveredByRows.begin(),
                        chart.uncoveredCols[a].coveredByRows.end(),
                        chart.uncoveredCols[b].coveredByRows.begin(),
                        chart.uncoveredCols[b].coveredByRows.end())) {
                chart.uncoveredCols[a].covered = true;  // 더 쉬운 A를 지움
                changed = true;
                break;
            }
        }
    }
    return changed;
}

// 행 지배 줄이기: X가 덮는 (남은)열을 Y가 다 덮고(X⊆Y) 게다가 Y가 리터럴까지 더 적으면,
//   X는 Y한테 모든 면에서 밀리니까 지운다.
//   리터럴 적은 정도는 popcount(mask)로 비교(돈케어 많을수록 리터럴 적음). 단 strict '>'.
//   동률이면 둘 다 최적해에 쓰일 수 있어서 함부로 못 지운다(동률 해 다 살려야 하니까).
static bool reduceRowDominance(ChartResult& chart) {
    bool changed = false;
    int rowSize = chart.remainingRows.size();

    // 비교는 "아직 안 덮인 열"만 대상으로 한다. 그 열들만 1로 세운 마스크를 미리 만들어 둠.
    Bits uncoveredMask = 0;
    for (const auto& c : chart.uncoveredCols)
        if (!c.covered)
            uncoveredMask |= (1ULL << c.mintermValue);

    for (int x = 0; x < rowSize; x++) {
        if (chart.remainingRows[x].selected) continue;
        Bits xCov = chart.remainingRows[x].term.coveredMinterms & uncoveredMask;  // X가 덮는 '남은' 열만
        for (int y = 0; y < rowSize; y++) {
            if (x == y || chart.remainingRows[y].selected) continue;
            Bits yCov = chart.remainingRows[y].term.coveredMinterms & uncoveredMask;
            if (isSubset(xCov, yCov) &&                                    // Y가 X를 다 덮고
                __builtin_popcountll(chart.remainingRows[y].term.mask)     // Y가 리터럴도 더 적으면
                    > __builtin_popcountll(chart.remainingRows[x].term.mask)) {
                chart.remainingRows[x].selected = true;  // 밀리는 X를 지움
                changed = true;
                break;
            }
        }
    }
    return changed;
}

// 결과로 넘길 차트 만들기: 골라진 행(selected)·끝난 열(covered)은 빼고,
//   남은 행 기준으로 coveredByRows를 다시 매겨서 돌려준다.
static ChartResult buildResult(const ChartResult& chart) {
    ChartResult result;
    result.confirmedEPI = chart.confirmedEPI;

    for (const auto& row : chart.remainingRows)
        if (!row.selected)                          // EPI로 확정됐거나 지워진 행은 빼고
            result.remainingRows.push_back(row);

    // 행을 솎으면 번호가 밀려서 옛날 coveredByRows가 안 맞는다. 그래서 새 행 기준으로 싹 다시 매긴다.
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

    // EPI 뽑기 / 열 지배 / 행 지배를 한 바퀴씩 돌리는데, 더 바뀔 게 없을 때까지 계속 반복한다.
    //   (하나를 줄이면 또 새 EPI가 보이거나 새 지배가 생기니까. 이게 멈추면 그게 cyclic core)
    // '|='라서 셋 다 매번 실행되고, 셋 중 하나라도 뭔가 바꿨으면 changed가 true → 한 바퀴 더.
    bool changed;
    do {
        changed = false;
        changed |= confirmEssentialPIs(chart);
        changed |= reduceColumnDominance(chart);
        changed |= reduceRowDominance(chart);
    } while (changed);

    return buildResult(chart);
}
