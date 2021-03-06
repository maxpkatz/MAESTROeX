#include <Maestro.H>
#include <Maestro_F.H>

using namespace amrex;

void 
Maestro::AdvectBaseDens(BaseState<Real>& rho0_predicted_edge)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::AdvectBaseDens()", AdvectBaseDens); 
    
    rho0_predicted_edge.setVal(0.0);

    if (!spherical) {
        AdvectBaseDensPlanar(rho0_predicted_edge);
        RestrictBase(rho0_new, true);
        FillGhostBase(rho0_new, true);
    } else {
        AdvectBaseDensSphr(rho0_predicted_edge);
    }
}

void 
Maestro::AdvectBaseDensPlanar(BaseState<Real>& rho0_predicted_edge_state)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::AdvectBaseDensPlanar()", AdvectBaseDensPlanar); 

    BaseState<Real> force(base_geom.max_radial_level+1,base_geom.nr_fine);

    // zero the new density so we don't leave a non-zero density in fine radial
    // regions that no longer have a corresponding full state
    std::fill(rho0_new.begin(), rho0_new.end(), 0.0);

    const int max_lev = base_geom.max_radial_level+1;
    auto rho0_predicted_edge = rho0_predicted_edge_state.array();
    auto force_arr = force.array();

    // Predict rho_0 to vertical edges

    for (int n = 0; n <= base_geom.max_radial_level; ++n) {

        Real * AMREX_RESTRICT rho0_old_p = rho0_old.dataPtr(); 
        Real * AMREX_RESTRICT w0_p = w0.dataPtr();  

        const Real dr_lev = base_geom.dr(n);

        for (int i = 1; i <= base_geom.numdisjointchunks(n); ++i) {

            int lo = base_geom.r_start_coord(n,i);
            int hi = base_geom.r_end_coord(n,i);

            AMREX_PARALLEL_FOR_1D(hi-lo+1, j, {
                int r = j + lo;
                int p = n+max_lev*r;

                force_arr(n,r) = -rho0_old_p[p] * (w0_p[n+max_lev*(r+1)] - w0_p[p]) / dr_lev;
            });
            amrex::Gpu::synchronize();
        }
    }

    MakeEdgeState1d(rho0_old, rho0_predicted_edge_state, force);

    for (int n = 0; n <= base_geom.max_radial_level; ++n) {

        Real * AMREX_RESTRICT rho0_old_p = rho0_old.dataPtr(); 
        Real * AMREX_RESTRICT rho0_new_p = rho0_new.dataPtr(); 
        Real * AMREX_RESTRICT w0_p = w0.dataPtr();  

        const Real dr_lev = base_geom.dr(n);
        const Real dt_loc = dt;
        
        for (int i = 1; i <= base_geom.numdisjointchunks(n); ++i) {

            int lo = base_geom.r_start_coord(n,i);
            int hi = base_geom.r_end_coord(n,i);

            AMREX_PARALLEL_FOR_1D(hi-lo+1, j, {
                int r = j + lo;
                int p = n+max_lev*r;

                rho0_new_p[p] = rho0_old_p[p] 
                    - dt_loc/dr_lev * (rho0_predicted_edge(n,r+1)*w0_p[n+max_lev*(r+1)] - rho0_predicted_edge(n,r)*w0_p[p]);
            });
            amrex::Gpu::synchronize();
        }
    }
}

void 
Maestro::AdvectBaseDensSphr(BaseState<Real>& rho0_predicted_edge_state)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::AdvectBaseDensSphr()", AdvectBaseDensSphr);

    const Real dr0 = base_geom.dr(0);
    const Real dtdr = dt / dr0;
    BaseState<Real> force(1,base_geom.nr_fine);
    const int max_lev = base_geom.max_radial_level+1;
    auto rho0_predicted_edge = rho0_predicted_edge_state.array();

    // Predict rho_0 to vertical edges
    Real * AMREX_RESTRICT rho0_old_p = rho0_old.dataPtr(); 
    Real * AMREX_RESTRICT rho0_new_p = rho0_new.dataPtr();
    Real * AMREX_RESTRICT w0_p = w0.dataPtr(); 
    const auto& r_cc_loc = base_geom.r_cc_loc;
    const auto& r_edge_loc = base_geom.r_edge_loc;
    auto force_arr = force.array();

    AMREX_PARALLEL_FOR_1D(base_geom.nr_fine, r, {
        int p = max_lev*r;
        force_arr(0,r) = -rho0_old_p[p] * (w0_p[max_lev*(r+1)] - w0_p[p]) / dr0 - 
            rho0_old_p[p]*(w0_p[p] + w0_p[max_lev*(r+1)])/r_cc_loc(0,r);
    });
    Gpu::synchronize();

    MakeEdgeState1d(rho0_old, rho0_predicted_edge_state, force);

    AMREX_PARALLEL_FOR_1D(base_geom.nr_fine, r, {
        int p = max_lev*r;
        rho0_new_p[p] = rho0_old_p[p] - dtdr/(r_cc_loc(0,r)*r_cc_loc(0,r)) * 
            (r_edge_loc(0,r+1)*r_edge_loc(0,r+1) * rho0_predicted_edge(0,r+1) * w0_p[max_lev*(r+1)] - 
            r_edge_loc(0,r)*r_edge_loc(0,r) * rho0_predicted_edge(0,r) * w0_p[p]);
    });
    Gpu::synchronize();
}

void 
Maestro::AdvectBaseEnthalpy(BaseState<Real>& rhoh0_predicted_edge)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::AdvectBaseEnthalpy()", AdvectBaseEnthalpy); 
    
    rhoh0_predicted_edge.setVal(0.0);

    if (spherical == 0) {
        AdvectBaseEnthalpyPlanar(rhoh0_predicted_edge);
        RestrictBase(rhoh0_new, true);
        FillGhostBase(rhoh0_new, true);
    } else {
        AdvectBaseEnthalpySphr(rhoh0_predicted_edge);
    }
}

void 
Maestro::AdvectBaseEnthalpyPlanar(BaseState<Real>& rhoh0_predicted_edge_state)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::AdvectBaseEnthalpyPlanar()", AdvectBaseEnthalpyPlanar); 

    BaseState<Real> force(base_geom.max_radial_level+1, base_geom.nr_fine);

    // zero the new enthalpy so we don't leave a non-zero enthalpy in fine radial
    // regions that no longer have a corresponding full state
    std::fill(rhoh0_new.begin(), rhoh0_new.end(), 0);

    const int max_lev = base_geom.max_radial_level+1;
    auto rhoh0_predicted_edge = rhoh0_predicted_edge_state.array();
    auto force_arr = force.array();

    // Update (rho h)_0

    for (int n = 0; n <= base_geom.max_radial_level; ++n) {

        const Real * AMREX_RESTRICT rhoh0_old_p = rhoh0_old.dataPtr(); 
        const Real * AMREX_RESTRICT w0_p = w0.dataPtr();  
        const auto psi_arr = psi.array();  

        const Real dr_lev = base_geom.dr(n);

        for (int i = 1; i <= base_geom.numdisjointchunks(n); ++i) {

            int lo = base_geom.r_start_coord(n,i);
            int hi = base_geom.r_end_coord(n,i);

            // here we predict (rho h)_0 on the edges
            AMREX_PARALLEL_FOR_1D(hi-lo+1, j, {
                int r = j + lo;
                int p = n+max_lev*r;
                force_arr(n,r) = -rhoh0_old_p[p] * (w0_p[n+max_lev*(r+1)] - w0_p[p]) / dr_lev 
                    + psi_arr(n,r);
            });
            Gpu::synchronize();
        }
    }

    MakeEdgeState1d(rhoh0_old, rhoh0_predicted_edge_state, force);

    for (int n = 0; n <= base_geom.max_radial_level; ++n) {

        const Real * AMREX_RESTRICT rhoh0_old_p = rhoh0_old.dataPtr(); 
        Real * AMREX_RESTRICT rhoh0_new_p = rhoh0_new.dataPtr(); 
        const Real * AMREX_RESTRICT w0_p = w0.dataPtr();  
        const auto psi_arr = psi.array(); 

        const Real dr_lev = base_geom.dr(n);
        const Real dt_loc = dt;

        for (int i = 1; i <= base_geom.numdisjointchunks(n); ++i) {

            int lo = base_geom.r_start_coord(n,i);
            int hi = base_geom.r_end_coord(n,i);

            // update (rho h)_0
            AMREX_PARALLEL_FOR_1D(hi-lo+1, j, {
                int r = j + lo;
                int p = n+max_lev*r;
                rhoh0_new_p[p] = rhoh0_old_p[p] 
                    - dt_loc/dr_lev * (rhoh0_predicted_edge(n,r+1)*w0_p[n+max_lev*(r+1)] - rhoh0_predicted_edge(n,r)*w0_p[p]) + dt_loc * psi_arr(n,r);
            });
            Gpu::synchronize();
        }
    }
}

void 
Maestro::AdvectBaseEnthalpySphr(BaseState<Real>& rhoh0_predicted_edge_state)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::AdvectBaseEnthalpySphr()", AdvectBaseEnthalpySphr);

    const Real dr0 = base_geom.dr(0);
    const Real dtdr = dt / dr0;
    const Real dt_loc = dt;

    const int max_lev = base_geom.max_radial_level + 1;
    BaseState<Real> force(1,base_geom.nr_fine);
    auto rhoh0_predicted_edge = rhoh0_predicted_edge_state.array();

    // predict (rho h)_0 on the edges
    const Real * AMREX_RESTRICT rhoh0_old_p = rhoh0_old.dataPtr(); 
    Real * AMREX_RESTRICT rhoh0_new_p = rhoh0_new.dataPtr();
    Real * AMREX_RESTRICT w0_p = w0.dataPtr(); 
    const auto& r_cc_loc = base_geom.r_cc_loc;
    const auto& r_edge_loc = base_geom.r_edge_loc;
    const auto psi_arr = psi.array(); 
    const auto force_arr = force.array();

    AMREX_PARALLEL_FOR_1D(base_geom.nr_fine, r, {
        int p = max_lev*r;
        force_arr(0,r) = -rhoh0_old_p[p] * (w0_p[max_lev*(r+1)] - w0_p[p]) / dr0 - 
            rhoh0_old_p[p]*(w0_p[p] + w0_p[max_lev*(r+1)])/r_cc_loc(0,r) + psi_arr(0,r);
    });
    Gpu::synchronize();

    MakeEdgeState1d(rhoh0_old, rhoh0_predicted_edge_state, force);

    AMREX_PARALLEL_FOR_1D(base_geom.nr_fine, r, {
        int p = max_lev*r;

        rhoh0_new_p[p] = rhoh0_old_p[p] - dtdr/(r_cc_loc(0,r)*r_cc_loc(0,r)) * 
            (r_edge_loc(0,r+1)*r_edge_loc(0,r+1) * rhoh0_predicted_edge(0,r+1) * w0_p[max_lev*(r+1)] - 
            r_edge_loc(0,r)*r_edge_loc(0,r) * rhoh0_predicted_edge(0,r) * w0_p[p]) + 
            dt_loc * psi_arr(0,r);
    });
    Gpu::synchronize();
}
