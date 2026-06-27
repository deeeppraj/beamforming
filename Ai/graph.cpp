// graph.cpp
// =============================================================================
// Top-level AIE graph instantiation
// =============================================================================
#include "graph.h"

BeamformFrontendGraph beamform_graph;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main(void) {
    beamform_graph.init();
    beamform_graph.run(1);   // run one full iteration (1000 snapshots -> 1 R matrix)
    beamform_graph.end();
    return 0;
}
#endif
