
#include <Maestro.H>
#include <AMReX_VisMF.H>
using namespace amrex;


// initialize AMR data
// perform initial projection
// perform divu iters
// perform initial (pressure) iterations
void
Maestro::Init ()
{
    Print() << "Calling Init()" << endl;

    // fill in multifab and base state data
    InitData();

    // initial projection
    InitProj();

    // compute initial time step
    FirstDt();

    // divu iters
    DivuIter();

    // initial (pressure) iters
    InitIter();
}

// fill in multifab and base state data
void
Maestro::InitData ()
{
    Print() << "Calling InitData()" << endl;

    // read in model file and fill in s0_init and p0_init for all levels
    init_base_state(s0_init.dataPtr(),p0_init.dataPtr(),rho0_new.dataPtr(),
                    rhoh0_new.dataPtr(),p0_new.dataPtr(),tempbar.dataPtr(),max_level+1,
                    ZFILL(geom[0].ProbLo()));

    // calls AmrCore::InitFromScratch(), which calls a MakeNewGrids() function 
    // that repeatedly calls Maestro::MakeNewLevelFromScratch() to build and initialize
    InitFromScratch(t_new);

    // synchronize levels
    AverageDown(snew);
    AverageDown(unew);

    // free memory in s0_init and p0_init by swapping it
    // with an empty vector that will go out of scope
    Vector<Real> s0_swap, p0_swap;
    std::swap(s0_swap,s0_init);
    std::swap(p0_swap,p0_init);

    if (fix_base_state) {
        compute_cutoff_coords(rho0_new.dataPtr());
        // FIXME call make_grav_cell(grav_cell,rho0_new)
    }
    else {
        if (do_smallscale) {
            // first compute cutoff coordinates using initial density profile
            compute_cutoff_coords(rho0_new.dataPtr());
            // set rho0_new = rhoh0_new = 0.
            std::fill(rho0_new.begin(),  rho0_new.end(),  0.);
            std::fill(rhoh0_new.begin(), rhoh0_new.end(), 0.);
        }
        else {
            // set rho0 to be the average
            Average(snew,rho0_new,Rho);
            compute_cutoff_coords(rho0_new.dataPtr());

            // compute p0 with HSE
            // FIXME call make_grav_cell(grav_cell,rho0_new)
            // call enforce_HSE(rho0_old,p0_old,grav_cell)
            // call eos with r,p as input to recompute T,h
            // call makeTHfromRhoP(sold,p0_old,the_bc_tower%bc_tower_array,mla,dx)

            // set rhoh0 to be the average
            Average(snew,rhoh0_new,RhoH);
        }
    }

    if (plot_int > 0) {
        WritePlotFile(0);
    }
}

// During initialization of a simulation, Maestro::InitData() calls 
// AmrCore::InitFromScratch(), which calls 
// a MakeNewGrids() function that repeatedly calls this function to build 
// and initialize finer levels.  This function creates a new fine
// level that did not exist before by interpolating from the coarser level
// overrides the pure virtual function in AmrCore
void Maestro::MakeNewLevelFromScratch (int lev, Real time, const BoxArray& ba,
				       const DistributionMapping& dm)
{
    sold[lev]    .define(ba, dm,          Nscal, 0);
    snew[lev]    .define(ba, dm,          Nscal, 0);
    uold[lev]    .define(ba, dm, AMREX_SPACEDIM, 0);
    unew[lev]    .define(ba, dm, AMREX_SPACEDIM, 0);
    S_cc_old[lev].define(ba, dm,              1, 0);
    S_cc_new[lev].define(ba, dm,              1, 0);
    gpi[lev]     .define(ba, dm, AMREX_SPACEDIM, 0);
    dSdt[lev]    .define(ba, dm,              1, 0);

    if (lev > 0 && do_reflux) {
        flux_reg_s[lev].reset(new FluxRegister(ba, dm, refRatio(lev-1), lev, Nscal));
        flux_reg_u[lev].reset(new FluxRegister(ba, dm, refRatio(lev-1), lev, AMREX_SPACEDIM));
    }

    const Real* dx = geom[lev].CellSize();
    Real cur_time = t_new;

    MultiFab& scal = snew[lev];
    MultiFab& vel = unew[lev];

    for (MFIter mfi(scal); mfi.isValid(); ++mfi)
    {
        const Box& box = mfi.validbox();
        const int* lo  = box.loVect();
        const int* hi  = box.hiVect();

        initdata(lev, cur_time, ARLIM_3D(lo), ARLIM_3D(hi),
                 BL_TO_FORTRAN_3D(scal[mfi]), 
                 BL_TO_FORTRAN_3D(vel[mfi]), 
                 s0_init.dataPtr(), p0_init.dataPtr(),
                 ZFILL(dx),ZFILL(geom[lev].ProbLo()));
    }
}


void Maestro::InitProj ()
{

    Vector<MultiFab>           S_cc(finest_level+1);
    Vector<MultiFab>   rho_omegadot(finest_level+1);
    Vector<MultiFab>        thermal(finest_level+1);
    Vector<MultiFab>       rho_Hnuc(finest_level+1);
    Vector<MultiFab>       rho_Hext(finest_level+1);

    for (int lev=0; lev<=finest_level; ++lev) {
        S_cc[lev].define        (grids[lev], dmap[lev],       1, 0);
        rho_omegadot[lev].define(grids[lev], dmap[lev], NumSpec, 0);
        thermal[lev].define     (grids[lev], dmap[lev],       1, 0);
        rho_Hnuc[lev].define    (grids[lev], dmap[lev],       1, 0);
        rho_Hext[lev].define    (grids[lev], dmap[lev],       1, 0);

        // during initial projection we ignore reaction terms
        rho_omegadot[lev].setVal(0.);
        rho_Hnuc[lev].setVal(0.);
    }

    ComputeHeating(rho_Hext);

    if (use_thermal_diffusion) {
        for (int lev=0; lev<=finest_level; ++lev) {
            thermal[lev].setVal(0.);   // FIXME
        }
    }
    else {
        for (int lev=0; lev<=finest_level; ++lev) {
            thermal[lev].setVal(0.);
        }
    }


    Make_S_cc(S_cc,snew,rho_omegadot,rho_Hnuc,rho_Hext,thermal);

}


void Maestro::DivuIter ()
{}


void Maestro::InitIter ()
{}
