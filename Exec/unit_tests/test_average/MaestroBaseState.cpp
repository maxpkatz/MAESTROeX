#include <Maestro.H>
#include <Maestro_F.H>

using namespace amrex;

void 
Maestro::InitBaseState(RealVector& rho0, RealVector& rhoh0, 
                       RealVector& p0, 
                       const int lev)
{
    // timer for profiling
    BL_PROFILE_VAR("Maestro::InitBaseState()", InitBaseState); 

    const int max_lev = base_geom.max_radial_level + 1;
    const int n = lev;

    for (auto r = 0; r< base_geom.nr(n); ++r) {
        // height above the bottom of the domain
        Real dist = (Real(r) + 0.5) * base_geom.dr(n);

        s0_init[n+max_lev*(r+base_geom.nr_fine*Rho)] = exp(-dist*dist/0.1);

        p0_init[n+max_lev*r] = s0_init[n+max_lev*(r+base_geom.nr_fine*Rho)];
    }
}
