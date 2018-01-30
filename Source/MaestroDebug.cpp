
#include <Maestro.H>

using namespace amrex;


// print out the contents of a cell-centered Vector of MultiFabs
void
Maestro::PrintMF (const Vector<MultiFab>& CC)
{
    for (int lev=0; lev<=finest_level; ++lev) {

        // get references to the MultiFabs at level lev
        const MultiFab& CC_mf = CC[lev];

        const BoxArray& ba = CC_mf.boxArray();
        const DistributionMapping& dm = CC_mf.DistributionMap();
        const int myProc = ParallelDescriptor::MyProc();

        for (int i=0; i<ba.size(); ++i) {
            if (dm[i] == myProc) {

                // we want all processors to write, not just the IOProcessor
                std::cout << "Grid #" << i << endl;
                std::cout << "Processor #" << myProc << endl;

                const Box& validBox = ba[i];

                // call fortran subroutine
                // use macros in AMReX_ArrayLim.H to pass in each FAB's data, 
                // lo/hi coordinates (including ghost cells), and/or the # of components
                // We will also pass "validBox", which specifies the "valid" region.
                print_mf(lev, ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                         BL_TO_FORTRAN_FAB(CC_mf[i]));

            }
            // add this barrier so only one grid gets printed out at a time
            ParallelDescriptor::Barrier();
        }
    }
}

void
Maestro::PrintEdge (const Vector<std::array< MultiFab, AMREX_SPACEDIM > >& EDGE,
                    int dir)
{
    for (int lev=0; lev<=finest_level; ++lev) {

        // get references to the MultiFabs at level lev
        const MultiFab& EDGE_mf = EDGE[lev][dir];

        const BoxArray& ba = EDGE_mf.boxArray();
        const DistributionMapping& dm = EDGE_mf.DistributionMap();
        const int myProc = ParallelDescriptor::MyProc();

        for (int i=0; i<ba.size(); ++i) {
            if (dm[i] == myProc) {

                // we want all processors to write, not just the IOProcessor
                std::cout << "Grid #" << i << endl;
                std::cout << "Processor #" << myProc << endl;

                // EDGE BASED
                const Box& validBox = ba[i];

                // call fortran subroutine
                // use macros in AMReX_ArrayLim.H to pass in each FAB's data, 
                // lo/hi coordinates (including ghost cells), and/or the # of components
                // We will also pass "validBox", which specifies the "valid" region.
                print_edge(lev, ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                           BL_TO_FORTRAN_FAB(EDGE_mf[i]));

            }
            // add this barrier so only one grid gets printed out at a time
            ParallelDescriptor::Barrier();
        }
    }
}
