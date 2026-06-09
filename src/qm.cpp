// Prime Implicant 유도 (김태현)
// 병합 공식은 README의 qm 항목에.

#include "qm.h"

std::vector<CombinedTerm> generatePI(
    const std::vector<int>& minterms,
    const std::vector<int>& dontCares,
    int numVars
) {
    std::vector<CombinedTerm> now;

    for(int i : minterms){
        CombinedTerm newterm;
        newterm.value=(Bits)i;//비트로 치환해서 넣기
        newterm.mask=0;//처음이니까 0
        newterm.coveredMinterms=1ULL<<i;//i번째 비트에 표시함으로 i번째 민텀을 커버하고 있다
        newterm.used=false;//처음이니까 false
        now.push_back(newterm);//이걸
    }
    for(int j : dontCares){
        CombinedTerm newterm;
        newterm.value=(Bits)j;
        newterm.mask=0;
        newterm.coveredMinterms=0;
        newterm.used=false;
        now.push_back(newterm);
    }

    
    std::vector<CombinedTerm> realPI;
    bool mergePossible=true;//PI 병합 성공여부

    while(mergePossible){
        mergePossible=false;//일단 아무것도 안 합쳤으니까 false로

        std::vector<CombinedTerm> nextturn;//다음에 병합할 때 쓸 임시 저장소
        std::vector<std::vector<CombinedTerm>> groups(numVars + 1); //변수가 n개면 n+1개 가 필요함

        for (CombinedTerm& term : now) {
            int onesCount = (int)__builtin_popcountll(term.value);
            groups[onesCount].push_back(term); // 1의 개수와 똑같은 번호의 방으로 넣기
        }

        for(int i=0; i<numVars; i++){
            for (CombinedTerm& a : groups[i]) {
                for (CombinedTerm& b : groups[i + 1]){
                        if(a.mask==b.mask){
                            Bits diff = (a.value ^ b.value) & ~a.mask & ~b.mask;
                            bool canMerge = (a.mask == b.mask)// don't care 자리가 같고
                            && (diff != 0)                    // 다른 자리가 있고
                            && ((diff & (diff - 1)) == 0);    // 그게 정확히 1비트

                            if(canMerge){
                                CombinedTerm merged;
                                merged.mask  = a.mask | diff;            // 달랐던 그 자리가 새 don't care
                                merged.value = a.value & ~merged.mask;   // don't care 자리는 0으로 맞춰둠
                                merged.coveredMinterms = a.coveredMinterms | b.coveredMinterms;
                                merged.used=false;
                                
                                bool TwinsPossible=false;//쌍둥이 가능성(nexttrun에 merged한게 들어있는지)
                                for(CombinedTerm& mirror: nextturn){
                                    if(mirror.value==merged.value&& mirror.mask==merged.mask){
                                        mirror.coveredMinterms |=merged.coveredMinterms;
                                        TwinsPossible=true;// 반복문에서 이 구문이 있는 이상 같은 놈이 자신 포함 3명 이상이 발견 될 수 없기 때문에 한번만 찾으면 끝
                                        break;//더 이상 볼 필요 없으니까 탈출
                                    }
                                }
                                if(TwinsPossible==false){//똑같은게 없어야 새로운 거니까 추가
                                    nextturn.push_back(merged);
                                }
                                a.used=true;//병합에 성공했으니까 a,b둘다 used true로 만듬
                                b.used=true;
                                mergePossible=true;//병합에 성공했으니까 또 되는 거 있는지 찾아야함
                            }
                        } 
                }            
            }
        }
        //짝이 없는 PI들을 찾아야함
        for(int i=0; i<=numVars; i++){
            for(CombinedTerm& solo: groups[i]){
                if(solo.used==false && solo.coveredMinterms>0){//한번도 병합 안당하고, 커버하는 민텀이 1이상이어야 PI로 껴줌
                    bool TwinsPossible=false;//쌍둥이 가능성
                    for(CombinedTerm& mirror: realPI){//아까 위랑 똑같은 상황
                        if(mirror.value==solo.value&& mirror.mask==solo.mask){
                            mirror.coveredMinterms |=solo.coveredMinterms;
                            TwinsPossible=true;
                            break;
                        }
                    } 
                    if(TwinsPossible==false){
                        realPI.push_back(solo);
                    }           
                }
            }
        }
        now=nextturn;
    }
    // TODO(김태현): 비트 수로 그룹 나누고, 인접 그룹끼리 1비트 차이 항을 반복 병합.
    //              끝까지 안 합쳐진 항을 모아서 반환.
    
    return realPI;
}
