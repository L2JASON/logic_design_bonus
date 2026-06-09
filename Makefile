# QM 최소화 프로그램 빌드 (5조)
#
#   make            전체 빌드 -> 실행파일 qm
#   make run        빌드 후 대화형 메뉴 실행
#   make clean      빌드 산출물 삭제
#
# 컴파일러는 g++ 기준. popcount에 __builtin_popcountll 을 써서 GCC/Clang 전용이다.
# MSVC를 쓰면 __popcnt64 등으로 바꿔야 한다(README 참고).

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -O2
SRCDIR   := src
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
HEADERS  := $(wildcard $(SRCDIR)/*.h)

# Windows(cmd 계열)와 Unix에서 실행파일 이름/삭제 명령이 달라 분기한다.
ifeq ($(OS),Windows_NT)
    TARGET := qm.exe
    RM     := del /Q
else
    TARGET := qm
    RM     := rm -f
endif

# 기본 목표: 실행파일 빌드
all: $(TARGET)

# 헤더가 바뀌어도 다시 빌드되도록 HEADERS를 의존성에 넣는다(모듈이 적어 통째로 컴파일).
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# 빌드 후 인자 없이 실행하면 대화형 메뉴가 뜬다.
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: all run clean
