#include <iostream>
#include <vector>
#include "src/types.h"
#include "src/search.h"

int main() {
    // Set up a simple Cyclic Core scenario
    ChartResult chart;
    
    // Remaining Prime Implicants (PIs)
    // PI 0: covers minterms m1, m2 (0b0110 = 6)
    // PI 1: covers minterms m2, m3 (0b1100 = 12)
    // PI 2: covers minterms m1, m3 (0b1010 = 10)
    
    CombinedTerm term0 = {0, 0, 6, false};
    CombinedTerm term1 = {0, 0, 12, false};
    CombinedTerm term2 = {0, 0, 10, false};
    
    chart.remainingRows.push_back({term0, false});
    chart.remainingRows.push_back({term1, false});
    chart.remainingRows.push_back({term2, false});
    
    // Uncovered Minterms
    // m1, m2, m3 need to be covered
    MintermColumn col_m1;
    col_m1.mintermValue = 1;
    col_m1.coveredByRows = {0, 2}; // PI 0 and PI 2 cover m1
    col_m1.covered = false;
    
    MintermColumn col_m2;
    col_m2.mintermValue = 2;
    col_m2.coveredByRows = {0, 1}; // PI 0 and PI 1 cover m2
    col_m2.covered = false;
    
    MintermColumn col_m3;
    col_m3.mintermValue = 3;
    col_m3.coveredByRows = {1, 2}; // PI 1 and PI 2 cover m3
    col_m3.covered = false;
    
    chart.uncoveredCols.push_back(col_m1);
    chart.uncoveredCols.push_back(col_m2);
    chart.uncoveredCols.push_back(col_m3);
    
    std::cout << "Running solveCyclicCore..." << std::endl;
    
    // Call the function
    std::vector<SOPCandidate> result = solveCyclicCore(chart, 4);
    
    std::cout << "solveCyclicCore finished." << std::endl;
    std::cout << "Number of candidates found: " << result.size() << std::endl;
    
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << "Candidate " << i << " has " << result[i].selectedPIs.size() << " PIs." << std::endl;
    }
    
    return 0;
}
