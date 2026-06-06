# QM 최소화 프로그램 (5조)

한동대 논리설계 보너스 과제. Quine-McCluskey로 Prime Implicant를 뽑고,
Branch & Bound로 최소 SOP(곱의 합) 식을 구하는 C++ 프로그램이다.

담당:
- 이준혁(조장) — 통합(`main`), PI Chart + EPI + 축소(`chart`)
- 김태현 — Prime Implicant 유도(`qm`)
- 이신형 — Cyclic Core 탐색(`search`)
- 방하영 — 입력 파싱 + 출력(`format`)


## 빌드 & 실행

```bash
g++ -std=c++17 -o qm src/*.cpp
```

```bash
./qm tests/input1.txt   # 파일에서 입력
./qm                    # 인자 없으면 키보드에서 입력
```

컴파일러는 `g++` 기준이다. popcount는 `__builtin_popcountll`을 쓰는데 이건
GCC/Clang 전용이다. 팀원 중 Visual Studio(MSVC)를 쓰면 컴파일이 안 되니,
그 경우 `__popcnt64` 또는 수동 popcount로 바꿔야 한다. (회의 때 확인)


## 파이프라인

```
parseInput          입력 5줄 -> InputData
   -> generatePI            (김태현)  PI 목록
   -> buildAndReduceChart   (이준혁)  EPI + 남은 행/열
   -> solveCyclicCore       (이신형)  최소 비용 해(들)
   -> printResult           (방하영)  SOP 식 + 비용 출력
```

단계 사이에 오가는 자료형은 전부 `src/types.h`에 있다. 이 파일이 4명의
공통 계약이라, 필드 이름이나 타입을 한 명이 바꾸면 나머지가 다 깨진다.
바꿀 일이 생기면 먼저 팀에 알릴 것.


## 설계 결정

### 비트 타입은 uint64_t로 통일

`int`은 크기가 플랫폼마다 다르고(최소 16비트 보장) 부호 비트 때문에 `>>`
동작도 제각각이다. `uint64_t`는 어디서나 정확히 64비트에 부호가 없어, 64칸을
전부 데이터로 쓸 수 있다. 게다가 Example1이 변수 6개라 minterm 번호가 0~63까지
나오는데, `coveredMinterms`에 63번 비트까지 세워야 해서 32비트로는 모자란다.

시프트할 때는 `1ULL`을 써야 한다. `1 << 63`은 `1`이 `int`라서 UB가 되고,
`1ULL << 63`이라야 안전하다.

### 비트 좌표계가 두 종류라는 점 (제일 헷갈리는 부분)

| 필드 | 비트 위치가 가리키는 것 | 폭 |
|---|---|---|
| `value`, `mask` | 변수 번호 | 하위 n비트 |
| `coveredMinterms` | minterm 번호 | 0 ~ 2^n-1 |

예를 들어 n=4에서 항 `x1 x2' - x4`는 `value=1001`, `mask=0010`이다
(x3 자리가 don't care라 mask에 표시). 반면 어떤 항이 minterm 1, 5번을 덮으면
`coveredMinterms`의 1번·5번 비트가 선다. 두 좌표계를 섞어 쓰면 100% 버그다.

**자릿값 방향(`coveredMinterms`)**: 맨 오른쪽 비트(LSB)가 m0, 그 왼쪽으로
m1, m2 … 순서다. 즉 **minterm 번호 k가 곧 비트 위치 k**다.

```
비트 위치:  … b3 b2 b1 b0
minterm  :  … m3 m2 m1 m0      예) 0b1010 -> m1, m3 을 덮음
```

그래서 "minterm k를 덮는가" 판정은 `(coveredMinterms >> k) & 1ULL`이고,
반대로 비트마스크를 minterm 번호 목록으로 풀려면 LSB부터 한 칸씩 검사하면 된다
(`chart.cpp`의 `mintermsOf` 참고). 이 방향은 **고정 약속**이다 — 아래
"아직 안 정한 것"의 `value` x1 자리(MSB냐 LSB냐) 문제와는 별개이고,
`coveredMinterms`는 어느 모듈에서나 LSB=m0으로 본다.

우리가 실제로 쓰는 건 하위 n비트뿐이라, 상위 비트를 잘라낼 때 `makeNBitMask(n)`
(하위 n비트만 1인 마스크)을 쓴다.

### don't care 취급

PI를 병합할 때는 don't care도 minterm처럼 같이 쓴다(더 큰 항으로 묶이니까).
하지만 차트에서 "덮어야 할 대상"은 f=1 minterm뿐이다. 그래서 각 항의
`coveredMinterms`와 `MintermColumn`에는 don't care 번호를 넣지 않는다.
이렇게 해야 EPI 판정이 f=1 열에 대해서만 일어난다.

### 입력 포맷 (5줄)

```
1줄: 변수 개수 n
2줄: f=1 minterm 개수
3줄: don't care 개수
4줄: f=1 minterm 목록
5줄: don't care 목록
```

키보드든 파일이든 `parseInput(std::istream&)` 하나로 받는다.

### 비용 비교 순서

`productCount` -> `literalCount` -> `inverterCount` 순으로 작은 쪽이 더 좋은 해다.


## 모듈별 구현 메모

### qm — generatePI

두 항 a, b가 병합 가능한 조건은 "don't care 위치가 같고, 나머지 자리 중
딱 한 비트만 다를 때"다.

```cpp
Bits diff = (a.value ^ b.value) & ~a.mask & ~b.mask;
bool canMerge = (a.mask == b.mask)          // don't care 자리가 같고
             && (diff != 0)                 // 다른 자리가 있고
             && ((diff & (diff - 1)) == 0); // 그게 정확히 1비트
```

합칠 때:

```cpp
merged.mask  = a.mask | diff;            // 달랐던 그 자리가 새 don't care
merged.value = a.value & ~merged.mask;   // don't care 자리는 0으로 맞춰둠
merged.coveredMinterms = a.coveredMinterms | b.coveredMinterms;
```

여기서 `+`(덧셈)는 절대 쓰지 말 것. 자리올림이 생겨 엉뚱한 비트가 오염된다.
순수 비트 연산만 쓴다. 그리고 병합에 한 번도 안 쓰인(`used==false`) 항이 PI다.

속도는, 항을 1의 개수(popcount)로 그룹 지으면 바로 옆 그룹끼리만 비교하면
된다 — 1비트 차이는 1의 개수가 정확히 1 차이 나는 그룹에서만 나오니까.

### chart — buildAndReduceChart

1. PI마다 행, f=1 minterm마다 열을 만든다. 각 열에, 그 minterm을 덮는 PI들의
   인덱스를 적어둔다. 덮는지 판정은 `(row.coveredMinterms >> mintermValue) & 1`.
2. 어떤 열을 덮는 PI가 딱 하나면 그 PI는 EPI다. `confirmedEPI`에 넣고, 그 EPI가
   덮는 열은 전부 covered 처리.
3. covered된 열을 지우고 남은 행/열을 반환한다. (행 지배·열 지배로 더 줄이는 건
   선택 사항. 시간 없으면 생략해도 search가 답은 낸다.)

주의: `minterms` 인자엔 f=1만 들어와야 한다. don't care가 섞이면 EPI 판정이
틀어진다. 이건 main에서 보장하고 있다.

### search — solveCyclicCore

EPI로 다 못 덮은 나머지를, 최소 개수의 PI로 마저 덮는 문제다(minimum set cover).
DFS로 풀되:

- 지금까지 고른 product 수가 기존 최적 기록을 **넘으면** 그 가지는 버린다.
  여기서 `>=`가 아니라 `>`인 게 중요하다. 비용이 같은 다른 최적해도 살려야 하니까.
- 안 덮인 열 중 **덮을 수 있는 PI가 가장 적은 열**을 골라, 그 열을 덮는 PI들로만
  분기한다(MRV). 이렇게 열을 고정해두면 {A,B}와 {B,A}를 둘 다 탐색하는 순열 중복이
  생기지 않는다.
- 재귀 들어가기 전에 상태(고른 PI, 커버 상황)를 백업하고 나오면 되돌린다.
  커버 상황을 `Bits` 하나로 들고 다니면 백업·복구가 값 복사 한 번으로 끝난다.

비용은 이렇게 센다:

```cpp
productCount  = selectedPIs.size();
literalCount  = Σ (numVars - popcount(term.mask));               // don't care 자리 빼기
inverterCount = Σ popcount(~term.value & ~term.mask & nBitMask); // value=0, mask=0 인 자리
```

### format — parseInput / printResult / termToString

항을 문자열로 바꾸는 규칙(변수 자리마다):

- `mask` 비트가 1이면 그 변수는 생략 (don't care)
- `mask`가 0이고 `value`가 1이면 `xi`
- `mask`가 0이고 `value`가 0이면 `xi'`

출력은 이런 모양이다:

```
Minimized SOP:
f(x1, x2, x3, x4) = x3x4' + x2'x3' + x1'x2x4
Cost: product count = 3, literal count = 7
```


## 아직 안 정한 것 (회의 필요)

- `x1`을 `value`의 MSB(bit n-1)로 볼지 LSB(bit 0)로 볼지. 이게 어긋나면 식은
  맞는데 변수 번호만 뒤집혀 나온다. `termToString`이 이 약속에 의존한다.
- `inverterCount` 정의. 일단 `popcount(~value & ~mask & nBitMask)`로 잠정 합의.
- 비용이 같은 최적해가 여러 개일 때 전부 출력할지, 하나만 낼지.


## 디렉토리

```
qm-team5/
├── src/
│   ├── types.h        공통 자료형 (계약)
│   ├── main.cpp       파이프라인        (이준혁)
│   ├── qm.h / .cpp                      (김태현)
│   ├── chart.h / .cpp                   (이준혁)
│   ├── search.h / .cpp                  (이신형)
│   └── format.h / .cpp                  (방하영)
├── tests/
│   └── input1~3.txt
└── README.md
```
