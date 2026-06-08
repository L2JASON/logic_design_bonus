# chart 모듈 세부 구현 (`buildAndReduceChart`)

담당: 이준혁. PI Chart 구성 → EPI 확정 → 행/열 지배 축소 → cyclic core 반환.
공통 자료형은 [`src/types.h`](../src/types.h), 전체 파이프라인 개요는 [`README.md`](../README.md) 참고.

```cpp
ChartResult buildAndReduceChart(
    const std::vector<CombinedTerm>& pis,    // qm가 뽑은 PI 목록
    const std::vector<int>& minterms         // 덮어야 할 f=1 minterm (don't care 제외)
);
```

---

## 0. 좌표계 두 종류 (헷갈리면 100% 버그)

| 필드 | 비트 위치 의미 |
|---|---|
| `value`, `mask` | 변수 번호 (하위 n비트) |
| `coveredMinterms` | **minterm 번호** (LSB=m0, 즉 minterm k = 비트 k) |

chart는 거의 `coveredMinterms` 좌표계에서만 논다. "PI가 minterm k를 덮나?" 판정은
한 줄이다: `(term.coveredMinterms >> k) & 1ULL`.

---

## 1. 표 구성

`ChartResult chart` 내부 작업본을 만들고 행·열을 채운다.

- **행** `remainingRows[i]` = `{ pis[i], selected=false }`
- **열** `uncoveredCols[c]` = `{ mintermValue, coveredByRows, covered=false }`
  - `coveredByRows`: 이 minterm을 덮는 PI들의 **행 인덱스 리스트**. 행 0,1,2… 순서로
    `push_back`하므로 **항상 오름차순 정렬**(뒤에서 `std::includes`가 이걸 전제로 함).
  - 채우는 판정: `(remainingRows[row].term.coveredMinterms >> mintermValue) & 1ULL`

> 인덱스 주의: 열 비교는 **인덱스가 아니라 minterm 값**(`mintermValue`) 기준. minterm이
> `{2,5,7}`처럼 띄엄띄엄이면 인덱스≠값이라, 값으로 시프트해야 한다.

---

## 2. 축소 루프 (고정점 do-while)

```
do {
    foundNew = false;
    (a) EPI 확정
    (b) 열 지배 축소
    (c) 행 지배 축소
} while (foundNew);   // 무언가 바뀌면 한 번 더
```

`foundNew`는 **실제로 무언가 제거/확정한 순간에만** true. 한 바퀴 돌아 아무 변화가 없으면
종료(= 더 줄일 게 없는 cyclic core 도달).

### (a) EPI 확정

어떤 열의 `coveredByRows.size() == 1`이면 그 minterm은 유일한 PI만 덮으므로 그 PI는
필수(Essential)다.
- 그 행 `selected = true`, `confirmedEPI`에 `term` push
- 그 EPI가 덮는 **모든 열**을 `covered = true` 처리:
  `(epi.coveredMinterms >> col.mintermValue) & 1ULL`

### (b) 열 지배 축소

열 A를 덮는 PI 집합이 열 B의 것을 **포함**하면(A ⊇ B) A를 지운다(`A.covered=true`).
B만 덮으면 A는 자동으로 덮이므로 A는 잉여. **더 어려운(덮는 PI가 적은) 열 B를 남긴다.**

- 집합 = `coveredByRows`(정렬됨) → `std::includes(A.begin,A.end, B.begin,B.end)`가 곧 B⊆A
- 동일 집합이면 `covered` skip + `break` 덕에 **하나만** 지워진다(둘 다 지워지지 않음)
- ⚠️ **빈 열 주의**: `coveredByRows`가 빈 열(덮는 PI 0개)은 모든 열의 부분집합이라
  다른 열을 전부 지워버린다. 이런 열은 "덮을 수 없는 minterm" = 비정상 입력에서만 생긴다.
  정상 입력(모든 f=1을 어떤 PI가 덮음)을 전제로 한다.

### (c) 행 지배 축소

PI X가 덮는 (남은)열 집합이 PI Y에 **포함**되고(X ⊆ Y) **Y가 X보다 비싸지 않으면** X를 지운다.
**더 많이 덮는(강한) PI Y를 남긴다.** 열 지배와 지우는 방향이 반대인 점에 주의.

비트 연산으로 처리한다:
```cpp
Bits uncoveredMask = (안 덮인 열들의 mintermValue 비트를 OR)
Bits xCov = X.coveredMinterms & uncoveredMask;   // 남은 열만
Bits yCov = Y.coveredMinterms & uncoveredMask;
if ((xCov & yCov) == xCov) { ... }               // X ⊆ Y
```
`uncoveredMask`는 열이 바뀌면 달라지므로 **매 패스 새로 만든다.**

**비용 게이트 (strict `>`)** — 이게 "전부 출력"의 핵심 안전장치:
```cpp
if (popcount(Y.mask) > popcount(X.mask)) { X 삭제; }
```
- 비용(리터럴 수) = `numVars - popcount(mask)`. 두 PI 비교 시 `numVars`가 상쇄돼
  `popcount(mask)` 비교만 남는다. don't-care 많을수록(popcount 큼) 싼 항.
- **`>=`가 아니라 `>`**: 동률(리터럴 같음)일 때 X를 지우면, X를 쓰는 다른 최적해가
  사라진다. chart에서 PI를 지우면 search가 영영 못 보므로 되돌릴 수 없다. 그래서
  "Y가 **엄격히** 더 쌀 때만" 지운다 → 최적해를 하나도 잃지 않는다. (팀 결정: 전부 출력)
- popcount은 `__builtin_popcountll` 사용(C++17 + GCC. `std::popcount`는 C++20이라 못 씀).

---

## 3. 결과 차트 반환

작업본 `chart`에서 살아남은 것만 새 `result`로 옮긴다.

1. `remainingRows` ← `selected == false`인 행만 (EPI·지배로 제거된 행 제외)
2. `uncoveredCols` ← `covered == false`인 열만
3. **각 열의 `coveredByRows`를 재계산** — 행을 솎으면서 인덱스가 밀리므로, 새
   `remainingRows`를 기준으로 다시 채운다(1번 표 구성과 같은 판정식). 재계산을 빼면
   `coveredByRows`가 옛 인덱스를 가리켜 범위 밖 접근이 난다.

```
remainingRows: [PI0,PI1,PI2,PI3] --(PI0,PI3 제거)--> [PI1,PI2]
               인덱스 0 1 2 3                          0  1
coveredByRows {1,2} 는 새 배열에선 {0,1} 로 다시 매겨야 함
```

---

## 4. 설계 메모

- **`selected` 한 플래그로 EPI·지배 제거 둘 다 표시**: 의미는 다르지만(해 채택 vs 폐기),
  "해에 포함됨"은 `confirmedEPI` 멤버십으로 판별하고 출력 필터는 `!selected`만 보므로
  기능상 안전. 깔끔함을 원하면 지역 `removed` 배열로 분리 가능.
- **search와의 계약**: 반환된 `remainingRows`/`uncoveredCols`는 이미 솎인 상태(제거된
  행·열 없음)이고, `coveredByRows`는 **반환된 `remainingRows` 기준 인덱스**다.

---

## 5. 테스트

qm/format이 아직 스텁이라, chart만 단독으로 빌드해 검증할 수 있다. PI별
`coveredMinterms`(와 비용용 `mask`)를 직접 지정해 `buildAndReduceChart`를 호출하고,
반환된 `confirmedEPI` / `remainingRows` / `uncoveredCols`를 확인하면 된다.

검증해 둔 케이스:
- **EPI 전부 해결**: 모든 열이 단독으로 덮여 `uncoveredCols`가 비고 `confirmedEPI`만 참.
- **열 지배**: 한 열의 coverer가 다른 열을 포함 → 포함하는 열이 사라짐.
- **행 지배 + 비용 게이트**: 부분집합 PI가 더 싼 PI에 지배되어 제거되고, 비싼 PI에는
  지배당하지 않음(strict `>`가 막음).
- **cyclic core**: EPI·지배가 모두 없어 차트가 그대로 유지됨.
- **동률 리터럴**: 부분집합이라도 비용이 같으면 제거되지 않음(strict `>` → 전부 출력 보존).

확인 포인트: `coveredByRows`의 모든 인덱스가 `remainingRows` 범위 안인지(재계산 정합성).
