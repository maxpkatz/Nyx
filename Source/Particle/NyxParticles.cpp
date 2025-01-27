#include <iomanip>
#include <Nyx.H>
#include <Gravity.H>

#include <Prob.H>

using namespace amrex;

namespace
{
    bool virtual_particles_set = false;
    
    std::string ascii_particle_file;
    std::string binary_particle_file;
    std::string    sph_particle_file;

#ifdef AGN
    std::string agn_particle_file;
#endif

#ifdef NEUTRINO_PARTICLES
    std::string neutrino_particle_file;
#endif

  // const std::string chk_particle_file("DM");
    const std::string dm_chk_particle_file("DM");
    const std::string agn_chk_particle_file("AGN");
    const std::string npc_chk_particle_file("NPC");

    //
    // We want to call this routine when on exit to clean up particles.
    //

    //
    // Array of containers for all active particles
    //
    Vector<NyxParticleContainerBase*> ActiveParticles;
    //
    // Array of containers for all virtual particles
    //
    Vector<NyxParticleContainerBase*> VirtualParticles;
    //
    // Array of containers for all ghost particles
    //
    Vector<NyxParticleContainerBase*> GhostParticles;

    //
    // Containers for the real "active" Particles
    //
    DarkMatterParticleContainer* DMPC = 0;
    StellarParticleContainer*     SPC = 0;
#ifdef AGN
    AGNParticleContainer*         APC = 0;
#endif
#ifdef NEUTRINO_PARTICLES

#ifdef NEUTRINO_DARK_PARTICLES
  DarkMatterParticleContainer*    NPC = 0;
#else
  NeutrinoParticleContainer*    NPC = 0;
#endif
  
#endif

    //
    // This is only used as a temporary container for
    //  reading in SPH particles and using them to
    //  initialize the density and velocity field on the
    //  grids.
    //
    DarkMatterParticleContainer*        SPHPC = 0;
    //
    // Container for temporary, virtual Particles
    //
    DarkMatterParticleContainer* VirtPC  = 0;
    StellarParticleContainer*    VirtSPC = 0;
#ifdef AGN
    AGNParticleContainer*        VirtAPC = 0;
#endif
#ifdef NEUTRINO_PARTICLES

#ifdef NEUTRINO_DARK_PARTICLES
    DarkMatterParticleContainer*  VirtNPC = 0;
#else
    NeutrinoParticleContainer*   VirtNPC = 0;

#endif
#endif
    //
    // Container for temporary, ghost Particles
    //
    DarkMatterParticleContainer* GhostPC  = 0;
    StellarParticleContainer*    GhostSPC = 0;
#ifdef AGN
    AGNParticleContainer*        GhostAPC = 0;
#endif
#ifdef NEUTRINO_PARTICLES
#ifdef NEUTRINO_DARK_PARTICLES
    DarkMatterParticleContainer*  GhostNPC = 0;
#else
    NeutrinoParticleContainer*   GhostNPC = 0;
#endif  
#endif

    void RemoveParticlesOnExit ()
    {
        for (int i = 0; i < ActiveParticles.size(); i++)
        {
            delete ActiveParticles[i];
            ActiveParticles[i] = 0;
        }
        for (int i = 0; i < GhostParticles.size(); i++)
        {
            delete GhostParticles[i];
            GhostParticles[i] = 0;
        }
        for (int i = 0; i < VirtualParticles.size(); i++)
        {
            delete VirtualParticles[i];
            VirtualParticles[i] = 0;
        }
    }
}

bool Nyx::do_dm_particles = false;
int Nyx::num_particle_ghosts = 1;
int Nyx::particle_skip_factor = 1;

std::string Nyx::particle_init_type = "";

// Allows us to output particles in the plotfile
//   in either single (IEEE32) or double (NATIVE) precision.  
// Particles are always written in double precision
//   in the checkpoint files.

bool Nyx::particle_initrandom_serialize = false;
Real Nyx::particle_initrandom_mass;
long Nyx::particle_initrandom_count;
long Nyx::particle_initrandom_count_per_box;
int  Nyx::particle_initrandom_iseed;
Real Nyx::particle_inituniform_mass;
Real Nyx::particle_inituniform_vx;
Real Nyx::particle_inituniform_vy;
Real Nyx::particle_inituniform_vz;

int Nyx::particle_launch_ics            = 1;

int Nyx::particle_verbose               = 1;
int Nyx::write_particle_density_at_init = 0;
Real Nyx::particle_cfl = 0.5;
#ifdef NEUTRINO_PARTICLES
Real Nyx::neutrino_cfl = 0.5;
#endif

Vector<NyxParticleContainerBase*>&
Nyx::theActiveParticles ()
{
    return ActiveParticles;
}

Vector<NyxParticleContainerBase*>&
Nyx::theGhostParticles ()
{
    return GhostParticles;
}

Vector<NyxParticleContainerBase*>&
Nyx::theVirtualParticles ()
{
    return VirtualParticles;
}

DarkMatterParticleContainer*
Nyx::theDMPC ()
{
    return DMPC;
}

DarkMatterParticleContainer*
Nyx::theVirtPC ()
{
    return VirtPC;
}
DarkMatterParticleContainer*
Nyx::theGhostPC ()
{
    return GhostPC;
}

StellarParticleContainer* 
Nyx::theSPC ()
{
      return SPC;
}
StellarParticleContainer* 
Nyx::theVirtSPC ()
{
      return VirtSPC;
}
StellarParticleContainer* 
Nyx::theGhostSPC ()
{
      return GhostSPC;
}

#ifdef AGN
AGNParticleContainer* 
Nyx::theAPC ()
{
      return APC;
}
AGNParticleContainer* 
Nyx::theVirtAPC ()
{
      return VirtAPC;
}
AGNParticleContainer* 
Nyx::theGhostAPC ()
{
      return GhostAPC;
}
#endif

#ifdef NEUTRINO_PARTICLES
#ifdef NEUTRINO_DARK_PARTICLES
DarkMatterParticleContainer* 
Nyx::theNPC ()
{
      return NPC;
}
DarkMatterParticleContainer* 
Nyx::theVirtNPC ()
{
      return VirtNPC;
}
DarkMatterParticleContainer* 
Nyx::theGhostNPC ()
{
      return GhostNPC;
}
#else
NeutrinoParticleContainer* 
Nyx::theNPC ()
{
      return NPC;
}
NeutrinoParticleContainer* 
Nyx::theVirtNPC ()
{
      return VirtNPC;
}
NeutrinoParticleContainer* 
Nyx::theGhostNPC ()
{
      return GhostNPC;
}
#endif
#endif

void
Nyx::read_particle_params ()
{
    ParmParse pp("nyx");
    pp.query("do_dm_particles", do_dm_particles);

#ifdef AGN
    pp.get("particle_init_type", particle_init_type);
#else
    if (do_dm_particles)
    {
        pp.get("particle_init_type", particle_init_type);
        pp.query("init_with_sph_particles", init_with_sph_particles);
    }
#endif

    pp.query("particle_initrandom_serialize", particle_initrandom_serialize);
    pp.query("particle_initrandom_count", particle_initrandom_count);
    pp.query("particle_initrandom_count_per_box", particle_initrandom_count_per_box);
    pp.query("particle_initrandom_mass", particle_initrandom_mass);
    pp.query("particle_initrandom_iseed", particle_initrandom_iseed);
    pp.query("particle_skip_factor", particle_skip_factor);
    pp.query("ascii_particle_file", ascii_particle_file);
    pp.query("particle_inituniform_mass", particle_inituniform_mass);
    pp.query("particle_inituniform_vx", particle_inituniform_vx);
    pp.query("particle_inituniform_vy", particle_inituniform_vy);
    pp.query("particle_inituniform_vz", particle_inituniform_vz);

    // Input error check
    if (do_dm_particles && !ascii_particle_file.empty() && particle_init_type != "AsciiFile")
    {
        if (ParallelDescriptor::IOProcessor())
            std::cerr << "ERROR::particle_init_type is not AsciiFile but you specified ascii_particle_file" << std::endl;;
        amrex::Error();
    }

    pp.query("sph_particle_file", sph_particle_file);

    // Input error check
    if (init_with_sph_particles != 1 && !sph_particle_file.empty())
    {
        if (ParallelDescriptor::IOProcessor())
            std::cerr << "ERROR::init_with_sph_particles is not 1 but you specified sph_particle_file" << std::endl;;
        amrex::Error();
    }

    // Input error check
    if (init_with_sph_particles == 1 && sph_particle_file.empty())
    {
        if (ParallelDescriptor::IOProcessor())
            std::cerr << "ERROR::init_with_sph_particles is 1 but you did not specify sph_particle_file" << std::endl;;
        amrex::Error();
    }

    pp.query("binary_particle_file", binary_particle_file);

    // Input error check
    if (!binary_particle_file.empty() && (particle_init_type != "BinaryFile" &&
                                          particle_init_type != "BinaryMetaFile" && 
                                          particle_init_type != "BinaryMortonFile"))
    {
        if (ParallelDescriptor::IOProcessor())
            std::cerr << "ERROR::particle_init_type is not BinaryFile, BinaryMetaFile, or BinaryMortonFile but you specified binary_particle_file" << std::endl;
        amrex::Error();
    }

#ifdef AGN
    pp.query("agn_particle_file", agn_particle_file);
    if (!agn_particle_file.empty() && particle_init_type != "AsciiFile")
    {
        if (ParallelDescriptor::IOProcessor())
            std::cerr << "ERROR::particle_init_type is not AsciiFile but you specified agn_particle_file" << std::endl;;
        amrex::Error();
    }
#endif

#ifdef NEUTRINO_PARTICLES
    pp.query("neutrino_particle_file", neutrino_particle_file);
    if (!neutrino_particle_file.empty() && (particle_init_type != "AsciiFile"&&
                                            particle_init_type != "BinaryMetaFile" && 
                                            particle_init_type != "BinaryFile" ))
        {
          if (ParallelDescriptor::IOProcessor())
            std::cerr << "ERROR::particle_init_type is not AsciiFile or BinaryFile but you specified neutrino_particle_file" << std::endl;;
          amrex::Error();
        }
#endif

    pp.query("write_particle_density_at_init", write_particle_density_at_init);
    //
    // Control the verbosity of the Particle class
    //
    ParmParse ppp("particles");
    ppp.query("v", particle_verbose);

    //
    // Set the cfl for particle motion (fraction of cell that a particle can
    // move in a timestep).
    //
    ppp.query("cfl", particle_cfl);
#ifdef NEUTRINO_PARTICLES
    ppp.query("neutrino_cfl", neutrino_cfl);
#endif
}

void
Nyx::init_particles ()
{
    BL_PROFILE("Nyx::init_particles()");

    if (level > 0)
        return;

    //
    // Need to initialize particles before defining gravity.
    //
    if (do_dm_particles)
    {
        BL_ASSERT (DMPC == 0);
        DMPC = new DarkMatterParticleContainer(parent);
        ActiveParticles.push_back(DMPC); 

        if (init_with_sph_particles == 1)
           SPHPC = new DarkMatterParticleContainer(parent);

        if (parent->subCycle())
        {
            VirtPC = new DarkMatterParticleContainer(parent);
            VirtualParticles.push_back(VirtPC); 

            GhostPC = new DarkMatterParticleContainer(parent);
            GhostParticles.push_back(GhostPC); }
        //
        // Make sure to call RemoveParticlesOnExit() on exit.
        //
        amrex::ExecOnFinalize(RemoveParticlesOnExit);
        //
        // 2 gives more stuff than 1.
        //
        DMPC->SetVerbose(particle_verbose);

        DarkMatterParticleContainer::ParticleInitData pdata = {{particle_initrandom_mass}, {}, {}, {}};

        if (particle_init_type == "Random")
        {
            if (particle_initrandom_count <= 0)
            {
                amrex::Abort("Nyx::init_particles(): particle_initrandom_count must be > 0");
            }
            if (particle_initrandom_iseed <= 0)
            {
                amrex::Abort("Nyx::init_particles(): particle_initrandom_iseed must be > 0");
            }

            if (verbose)
            {
                amrex::Print() << "\nInitializing DM with cloud of "
                               << particle_initrandom_count
                               << " random particles with initial seed: "
                               << particle_initrandom_iseed << "\n\n";
            }
            {
            amrex::Gpu::LaunchSafeGuard lsg(particle_launch_ics);
            DMPC->InitRandom(particle_initrandom_count,
                             particle_initrandom_iseed, pdata,
                             particle_initrandom_serialize);
            }

        }
        else if (particle_init_type == "RandomPerBox")
        {
            if (particle_initrandom_count_per_box <= 0)
            {
                amrex::Abort("Nyx::init_particles(): particle_initrandom_count_per_box must be > 0");
            }
            if (particle_initrandom_iseed <= 0)
            {
                amrex::Abort("Nyx::init_particles(): particle_initrandom_iseed must be > 0");
            }

            if (verbose)
                amrex::Print() << "\nInitializing DM with of " << particle_initrandom_count_per_box
                               << " random particles per box with initial seed: "
                               << particle_initrandom_iseed << "\n\n";

            DMPC->InitRandomPerBox(particle_initrandom_count_per_box,
                                   particle_initrandom_iseed, pdata);

        }
        else if (particle_init_type == "RandomPerCell")
        {
            if (verbose)
                amrex::Print() << "\nInitializing DM with 1 random particle per cell " << "\n";

            int n_per_cell = 1;
            amrex::Gpu::LaunchSafeGuard lsg(particle_launch_ics);
            DMPC->InitNRandomPerCell(n_per_cell, pdata);
            amrex::Gpu::Device::synchronize();

        }
        else if (particle_init_type == "OnePerCell")
        {
            if (verbose)
                amrex::Print() << "\nInitializing DM with 1 uniform particle per cell " << "\n";

            //            int n_per_cell = 1;
            DarkMatterParticleContainer::ParticleInitData pdata_vel = {{particle_inituniform_mass, particle_inituniform_vx, particle_inituniform_vy, particle_inituniform_vz},{},{},{}};
            DMPC->InitOnePerCell(0.5, 0.5, 0.5, pdata_vel);
        }
        else if (particle_init_type == "AsciiFile")
        {
            if (verbose)
            {
                amrex::Print() << "\nInitializing DM particles from \""
                               << ascii_particle_file << "\" ...\n\n";
                if (init_with_sph_particles == 1)
                    amrex::Print() << "\nInitializing SPH particles from ascii \""
                                   << sph_particle_file << "\" ...\n\n";
            }
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]`. Here we're reading in the particle
            // mass and velocity.
            //
            DMPC->InitFromAsciiFile(ascii_particle_file, AMREX_SPACEDIM + 1);
            if (init_with_sph_particles == 1)
                SPHPC->InitFromAsciiFile(sph_particle_file, AMREX_SPACEDIM + 1);
        }
        else if (particle_init_type == "BinaryFile")
        {
            if (verbose)
            {
                amrex::Print() << "\nInitializing DM particles from \""
                               << binary_particle_file << "\" ...\n\n";
                if (init_with_sph_particles == 1)
                    amrex::Print() << "\nInitializing SPH particles from binary \""
                                   << sph_particle_file << "\" ...\n\n";
            }
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]`. Here we're reading in the particle
            // mass and velocity.
            //
            amrex::Gpu::LaunchSafeGuard lsg(particle_launch_ics);
            DMPC->InitFromBinaryFile(binary_particle_file, AMREX_SPACEDIM + 1);
            if (init_with_sph_particles == 1)
              SPHPC->InitFromBinaryFile(sph_particle_file, AMREX_SPACEDIM + 1);

        }
        else if (particle_init_type == "BinaryMetaFile")
        {
            if (verbose)
            {
                amrex::Print() << "\nInitializing DM particles from meta file\""
                               << binary_particle_file << "\" ...\n\n";
                if (init_with_sph_particles == 1)
                    amrex::Print() << "\nInitializing SPH particles from meta file\""
                                   << sph_particle_file << "\" ...\n\n";
            }
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]` in each of the binary particle files.
            // Here we're reading in the particle mass and velocity.
            //
            amrex::Gpu::LaunchSafeGuard lsg(particle_launch_ics);
            DMPC->InitFromBinaryMetaFile(binary_particle_file, AMREX_SPACEDIM + 1);
            if (init_with_sph_particles == 1)
                SPHPC->InitFromBinaryMetaFile(sph_particle_file, AMREX_SPACEDIM + 1);
        }
        else if (particle_init_type == "BinaryMortonFile")
        {
          if (verbose)
            {
              amrex::Print() << "\nInitializing DM particles from morton-ordered binary file\""
                             << binary_particle_file << "\" ...\n\n";
              if (init_with_sph_particles == 1)
                amrex::Error("Morton-ordered input is not supported for sph particles.");
            }
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]` in each of the binary particle files.
            // Here we're reading in the particle mass and velocity.
            //
          DMPC->InitFromBinaryMortonFile(binary_particle_file,
                                         AMREX_SPACEDIM + 1,
                                         particle_skip_factor);
        }
        else
        {
            amrex::Error("not a valid input for nyx.particle_init_type");
        }
    }
#ifdef AGN
    {
        // Note that we don't initialize any actual AGN particles here, we just create the container.
        BL_ASSERT (APC == 0);
        APC = new AGNParticleContainer(parent, num_particle_ghosts);
        ActiveParticles.push_back(APC); 

        if (parent->subCycle())
        {
            VirtAPC = new AGNParticleContainer(parent, num_particle_ghosts);
            VirtualParticles.push_back(VirtAPC); 

            GhostAPC = new AGNParticleContainer(parent, num_particle_ghosts);
            GhostParticles.push_back(GhostAPC); 
        }
        //
        // 2 gives more stuff than 1.
        //
        APC->SetVerbose(particle_verbose);
    }
#endif
#ifdef NEUTRINO_PARTICLES
    {
        BL_ASSERT (NPC == 0);
#ifdef NEUTRINO_DARK_PARTICLES
        NPC = new DarkMatterParticleContainer(parent);
        ActiveParticles.push_back(NPC); 
#else
        NPC = new NeutrinoParticleContainer(parent);
        ActiveParticles.push_back(NPC); 

        // Set the relativistic flag to 1 so that the density will be gamma-weighted
        //     when returned by AssignDensity calls.
        NPC->SetRelativistic(1);

        // We must set the value for csquared which is used in computing gamma = 1 / sqrt(1-vsq/csq)
        // Obviously this value is just a place-holder for now.
        NPC->SetCSquared(1.);

#endif

        if (parent->subCycle())
        {
#ifdef NEUTRINO_DARK_PARTICLES
            VirtNPC = new DarkMatterParticleContainer(parent);
            VirtualParticles.push_back(VirtNPC); 
#else
            VirtNPC = new NeutrinoParticleContainer(parent);
            VirtualParticles.push_back(VirtNPC); 

            // Set the relativistic flag to 1 so that the density will be gamma-weighted
            //     when returned by AssignDensity calls.
            VirtNPC->SetRelativistic(1);

#endif

#ifdef NEUTRINO_DARK_PARTICLES
            GhostNPC = new DarkMatterParticleContainer(parent);
            GhostParticles.push_back(GhostNPC); 

#else
            GhostNPC = new NeutrinoParticleContainer(parent);
            GhostParticles.push_back(GhostNPC); 

            // Set the relativistic flag to 1 so that the density will be gamma-weighted
            //     when returned by AssignDensity calls.
            GhostNPC->SetRelativistic(1);
#endif      

        }
        //
        // Make sure to call RemoveParticlesOnExit() on exit.
        //   (if do_dm_particles then we have already called ExecOnFinalize)
        //
        if (!do_dm_particles)
            amrex::ExecOnFinalize(RemoveParticlesOnExit);
        //
        // 2 gives more stuff than 1.
        //
        NPC->SetVerbose(particle_verbose);
        if (particle_init_type == "AsciiFile")
        {
            if (verbose)
                amrex::Print() << "\nInitializing Neutrino particles from \""
                               << neutrino_particle_file << "\" ...\n\n";
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]`. Here we're reading in the particle
            // mass, velocity and angles.
            //
#ifdef NEUTRINO_DARK_PARTICLES
            NPC->InitFromAsciiFile(neutrino_particle_file, AMREX_SPACEDIM + 1);
#else
            NPC->InitFromAsciiFile(neutrino_particle_file, 2*AMREX_SPACEDIM + 1);
#endif
        }
        else if (particle_init_type == "BinaryFile")
        {
            if (verbose)
            {
                amrex::Print() << "\nInitializing Neutrino particles from \""
                               << neutrino_particle_file << "\" ...\n\n";
            }
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]`. Here we're reading in the particle
            // mass and velocity.
            //
            NPC->InitFromBinaryFile(neutrino_particle_file, AMREX_SPACEDIM + 1);
        }
        else if (particle_init_type == "BinaryMetaFile")
        {
            if (verbose)
            {
                amrex::Print() << "\nInitializing NPC particles from meta file\""
                               << neutrino_particle_file << "\" ...\n\n";
            }
            //
            // The second argument is how many Reals we read into `m_data[]`
            // after reading in `m_pos[]` in each of the binary particle files.
            // Here we're reading in the particle mass and velocity.
            //
            NPC->InitFromBinaryMetaFile(neutrino_particle_file, AMREX_SPACEDIM + 1);
        }
        else
        {
            amrex::Error("for right now we only init Neutrino particles with ascii or binary");
        }
    }

    if (write_particle_density_at_init == 1)
        {

          Vector<std::unique_ptr<MultiFab> > particle_mf;//(new MultiFab(grids,dmap,1,1));
            
            DMPC->AssignDensity(particle_mf,0,1,0,0);

            writeMultiFabAsPlotFile("ParticleDensity", *particle_mf[0], "density");

#ifdef NEUTRINO_PARTICLES
            Vector<std::unique_ptr<MultiFab> > particle_npc_mf;//(new MultiFab(grids,dmap,1,1));
          //      DMPC->AssignDensitySingleLevel(particle_mf,0,1,0,0);

            NPC->AssignDensity(particle_npc_mf,0,1,0,0);

            writeMultiFabAsPlotFile("ParticleNPCDensity", *particle_npc_mf[0], "density");
#endif
            exit(0);
        }

#endif    
}

#ifndef NO_HYDRO
void
Nyx::init_santa_barbara (int init_sb_vels)
{
    BL_PROFILE("Nyx::init_santa_barbara()");
    Real cur_time = state[State_Type].curTime();
    Real a = old_a;

    BL_PROFILE_VAR("Nyx::init_santa_barbara()::part", CA_part);
    amrex::Print() << "... time and comoving a when data is initialized at level " 
                   << level << " " << cur_time << " " << a << '\n';

    if (level == 0)
    {
        Real frac_for_hydro = comoving_OmB / comoving_OmM;
        Real omfrac = 1.0 - frac_for_hydro;
 
        if ( (init_with_sph_particles == 0) && (frac_for_hydro != 1.0) ) {
            DMPC->MultiplyParticleMass(level, omfrac);
        }

        Vector<std::unique_ptr<MultiFab> > particle_mf(1);
        if (init_sb_vels == 1)
        {
            if (init_with_sph_particles == 1) {
               SPHPC->AssignDensityAndVels(particle_mf);
            } else {
               DMPC->AssignDensityAndVels(particle_mf);
            }

        } else {
            if (init_with_sph_particles == 1) {
                SPHPC->AssignDensity(particle_mf);
            } else {
                DMPC->AssignDensity(particle_mf);
            }
        }

        // As soon as we have used the SPH particles to define the density
        //    and velocity on the grid, we can go ahead and destroy them.
        if (init_with_sph_particles == 1) {
            delete SPHPC;
        }

        BL_PROFILE_VAR_STOP(CA_part);
        BL_PROFILE_VAR("Nyx::init_santa_barbara()::avg", CA_avg);

        for (int lev = parent->finestLevel()-1; lev >= 0; lev--)
        {
            amrex::average_down(*particle_mf[lev+1], *particle_mf[lev],
                                 parent->Geom(lev+1), parent->Geom(lev), 0, 1,
                                 parent->refRatio(lev));
        }
        BL_PROFILE_VAR_STOP(CA_avg);
        BL_PROFILE_VAR("Nyx::init_santa_barbara()::partmf", CA_partmf);
        // Only multiply the density, not the velocities
        if (init_with_sph_particles == 0)
        {
            if (frac_for_hydro == 1.0)
            {
                particle_mf[level]->mult(0,0,1);
            }
            else
            {
                particle_mf[level]->mult(frac_for_hydro / omfrac,0,1);
            }
        }
        BL_PROFILE_VAR_STOP(CA_partmf);

        BL_PROFILE_VAR("Nyx::init_santa_barbara()::init", CA_init);
        const auto geomdata = geom.data();
        MultiFab& S_new = get_new_data(State_Type);
        MultiFab& D_new = get_new_data(DiagEOS_Type);

        // Temp unused for GammaLaw, set it here so that pltfiles have
        // defined numbers
        D_new.setVal(0, Temp_comp);
        D_new.setVal(0,   Ne_comp);

#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(S_new,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.validbox();
            const auto fab_S_new=S_new.array(mfi);
            const auto fab_D_new=D_new.array(mfi);

            GpuArray<amrex::Real,max_prob_param> prob_param;
            prob_param_fill(prob_param);
            prob_param_special_fill(prob_param);
            comoving_type=int(std::round(prob_param[comoving_type_comp]));

            prob_initdata_on_box(bx, fab_S_new, fab_D_new, geomdata, prob_param);

//          amrex::ParallelFor(
//                             bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) noexcept {
//                             prob_initdata
//                                 (i, j ,k, fab_S_new, fab_D_new, geomdata,prob_param);
//                             });
        }

        amrex::Gpu::Device::streamSynchronize();

        if (inhomo_reion) init_zhi();

        BL_PROFILE_VAR_STOP(CA_init);

        // Add the particle density to the gas density 
        MultiFab::Add(S_new, *particle_mf[level], 0, Density_comp, 1, 1);

        if (init_sb_vels == 1)
        {
            // Convert velocity to momentum
            for (int i = 0; i < AMREX_SPACEDIM; ++i) {
               MultiFab::Multiply(*particle_mf[level], *particle_mf[level], 0, 1+i, 1, 0);
            }

            // Add the particle momenta to the gas momenta (initially zero)
            MultiFab::Add(S_new, *particle_mf[level], 1, Xmom_comp, AMREX_SPACEDIM, S_new.nGrow());
        }

        enforce_minimum_density_floor(S_new, new_a);
    } else {

        MultiFab& S_new = get_new_data(State_Type);
        FillCoarsePatch(S_new, 0, cur_time, State_Type, 0, S_new.nComp());

        MultiFab& D_new = get_new_data(DiagEOS_Type);
        FillCoarsePatch(D_new, 0, cur_time, DiagEOS_Type, 0, D_new.nComp());
 
        MultiFab& Phi_new = get_new_data(PhiGrav_Type);
        FillCoarsePatch(Phi_new, 0, cur_time, PhiGrav_Type, 0, Phi_new.nComp());

#ifndef CONST_SPECIES
       // Convert (rho X)_i to X_i before calling init_e_from_T
       for (int i = 0; i < NumSpec; ++i) 
           MultiFab::Divide(S_new, S_new, Density_comp, FirstSpec_comp+i, 1, 0);
#endif
    }

    init_e_from_T(a);

#ifndef CONST_SPECIES
    // Convert X_i to (rho X)_i
    MultiFab& S_new = get_new_data(State_Type);
    for (int i = 0; i < NumSpec; ++i) 
        MultiFab::Multiply(S_new, S_new, Density_comp, FirstSpec_comp+i, 1, 0);
#endif
}
#endif

void
Nyx::particle_post_restart (const std::string& restart_file, bool is_checkpoint)
{
    BL_PROFILE("Nyx::particle_post_restart()");

    if (level > 0)
        return;
     
    if (do_dm_particles)
    {
        BL_ASSERT(DMPC == 0);
        DMPC = new DarkMatterParticleContainer(parent);
        ActiveParticles.push_back(DMPC);

        if (parent->subCycle())
        {
            VirtPC = new DarkMatterParticleContainer(parent);
            VirtualParticles.push_back(VirtPC);
 
            GhostPC = new DarkMatterParticleContainer(parent);
            GhostParticles.push_back(GhostPC);
        }

        //
        // Make sure to call RemoveParticlesOnExit() on exit.
        //
        amrex::ExecOnFinalize(RemoveParticlesOnExit);

        //
        // 2 gives more stuff than 1.
        //
        DMPC->SetVerbose(particle_verbose);

        {
          DMPC->Restart(restart_file, dm_chk_particle_file, is_checkpoint);
          amrex::Gpu::Device::streamSynchronize();
        }
        //
        // We want the ability to write the particles out to an ascii file.
        //
        ParmParse pp("particles");

        std::string dm_particle_output_file;
        pp.query("dm_particle_output_file", dm_particle_output_file);

        if (!dm_particle_output_file.empty())
        {
            DMPC->WriteAsciiFile(dm_particle_output_file);
        }
    }
#ifdef AGN
    {
        BL_ASSERT(APC == 0);
        APC = new AGNParticleContainer(parent, num_particle_ghosts);
        ActiveParticles.push_back(APC);
 
        if (parent->subCycle())
        {
          VirtAPC = new AGNParticleContainer(parent, num_particle_ghosts);
          VirtualParticles.push_back(VirtAPC);
 
          GhostAPC = new AGNParticleContainer(parent, num_particle_ghosts);
          GhostParticles.push_back(GhostAPC);
        }

        //
        // Make sure to call RemoveParticlesOnExit() on exit.
        //
        amrex::ExecOnFinalize(RemoveParticlesOnExit);
        //
        // 2 gives more stuff than 1.
        //
        APC->SetVerbose(particle_verbose);
        APC->Restart(restart_file, agn_chk_particle_file, is_checkpoint);
        //
        // We want the ability to write the particles out to an ascii file.
        //
        ParmParse pp("particles");

        std::string agn_particle_output_file;
        pp.query("agn_particle_output_file", agn_particle_output_file);

        if (!agn_particle_output_file.empty())
        {
            APC->WriteAsciiFile(agn_particle_output_file);
        }
    }
#endif

#ifdef NEUTRINO_DARK_PARTICLES
    {
        BL_ASSERT(NPC == 0);
        NPC = new DarkMatterParticleContainer(parent);
        ActiveParticles.push_back(NPC);
 
        if (parent->subCycle())
        {
          VirtNPC = new DarkMatterParticleContainer(parent);
          VirtualParticles.push_back(VirtNPC);
 
          GhostNPC = new DarkMatterParticleContainer(parent);
          GhostParticles.push_back(GhostNPC);
        }

        //
        // Make sure to call RemoveParticlesOnExit() on exit.
        //
        amrex::ExecOnFinalize(RemoveParticlesOnExit);
        //
        // 2 gives more stuff than 1.
        //
        NPC->SetVerbose(particle_verbose);
        NPC->Restart(restart_file, npc_chk_particle_file, is_checkpoint);
        //
        // We want the ability to write the particles out to an ascii file.
        //
        ParmParse pp("particles");

        std::string npc_particle_output_file;
        pp.query("npc_particle_output_file", npc_particle_output_file);

        if (!npc_particle_output_file.empty())
        {
            NPC->WriteAsciiFile(npc_particle_output_file);
        }
    }
#endif
}

void
Nyx::particle_est_time_step (Real& est_dt)
{
    BL_PROFILE("Nyx::particle_est_time_step()");
    const Real cur_time = state[PhiGrav_Type].curTime();
    const Real a = get_comoving_a(cur_time);
    MultiFab& grav = get_new_data(Gravity_Type);
    const Real est_dt_particle = DMPC->estTimestep(grav, a, level, particle_cfl);

    if (est_dt_particle > 0) {
        est_dt = std::min(est_dt, est_dt_particle);
    }

#ifdef NEUTRINO_PARTICLES
    const Real est_dt_neutrino = NPC->estTimestep(grav, a, level, neutrino_cfl);
    if (est_dt_neutrino > 0) {
        est_dt = std::min(est_dt, est_dt_neutrino);
    }
#endif

    if (verbose)
    {
        if (est_dt_particle > 0)
        {
            amrex::Print() << "...estdt from particles at level "
                      << level << ": " << est_dt_particle << '\n';
        }
        else
            {
            amrex::Print() << "...there are no particles at level "
                      << level << '\n';
        }
#ifdef NEUTRINO_PARTICLES
        if (est_dt_neutrino > 0)
        {
            amrex::Print() << "...estdt from neutrinos at level "
                      << level << ": " << est_dt_neutrino << '\n';
        }
#endif
    }
}

void
Nyx::particle_redistribute (int lbase, bool my_init)
{
    BL_PROFILE("Nyx::particle_redistribute()");
    if (DMPC)
    {


        //  
        // If we are calling with my_init = true, then we want to force the redistribute
        //    without checking whether the grids have changed.
        //  
        if (my_init)
        {
            DMPC->Redistribute(lbase);
            return;
        }

        //
        // These are usually the BoxArray and DMap from the last regridding.
        //
        static Vector<BoxArray>            ba;
        static Vector<DistributionMapping> dm;

        bool    changed      = false;
        bool dm_changed      = false;
        bool ba_changed      = false;
        bool ba_size_changed = false;

        int flev = parent->finestLevel();
        
        while ( parent->getAmrLevels()[flev] == nullptr ) {
            flev--;
        }
 
        if (ba.size() != flev+1)
        {
            amrex::Print() << "BA SIZE " << ba.size() << std::endl;
            amrex::Print() << "FLEV    " << flev << std::endl;
            ba.resize(flev+1);
            dm.resize(flev+1);
            changed = true;
            ba_size_changed = true;
        }
        else
        {
            for (int i = 0; i <= flev && !changed; i++)
            {
                if (ba[i] != parent->boxArray(i))
                {
                    //
                    // The BoxArrays have changed in the regridding.
                    //
                    changed = true;
                    ba_changed = true;
                }

                if ( ! changed)
                {
                    if (dm[i] != parent->getLevel(i).get_new_data(0).DistributionMap())
                    {
                        //
                        // The DistributionMaps have changed in the regridding.
                        //
                        changed = true;
                        dm_changed = true;
                    }
                }
            }
        }

        if (changed)
        {
            //
            // We only need to call Redistribute if the BoxArrays or DistMaps have changed.
            // We also only call it for particles >= lbase. This is
            // because of we called redistribute during a subcycle, there may be particles not in
            // the proper position on coarser levels.
            //
            if (verbose)
            {
                if (ba_size_changed)
                   amrex::Print() << "Calling redistribute because the size of BoxArray changed " << '\n';
                else if (ba_changed)
                   amrex::Print() << "Calling redistribute because BoxArray changed " << '\n';
                else if (dm_changed)
                   amrex::Print() << "Calling redistribute because DistMap changed " << '\n';
            }

            int iteration = 1;
            for (int i = 0; i < theActiveParticles().size(); i++)
              {
                  theActiveParticles()[i]->Redistribute(lbase,
                                                  theActiveParticles()[i]->finestLevel(),
                                                  iteration);

              }

            //
            // Use the new BoxArray and DistMap to define ba and dm for next time.
            //
            for (int i = 0; i <= flev; i++)
            {
                ba[i] = parent->boxArray(i);
                dm[i] = parent->getLevel(i).get_new_data(0).DistributionMap();
            }
        }
        else
        {
            if (verbose)
                amrex::Print() << "NOT calling redistribute because NOT changed " << '\n';
        }
    }
}

void
Nyx::setup_virtual_particles()
{
    BL_PROFILE("Nyx::setup_virtual_particles()");

    if(Nyx::theDMPC() != 0 && !virtual_particles_set)
    {
        DarkMatterParticleContainer::AoS virts;
        if (level < parent->finestLevel())
        {
            get_level(level + 1).setup_virtual_particles();
            Nyx::theVirtPC()->CreateVirtualParticles(level+1, virts);
            Nyx::theVirtPC()->AddParticlesAtLevel(virts, level);
            Nyx::theDMPC()->CreateVirtualParticles(level+1, virts);
            Nyx::theVirtPC()->AddParticlesAtLevel(virts, level);
        }
        virtual_particles_set = true;
    }
}

void
Nyx::remove_virtual_particles()
{
    BL_PROFILE("Nyx::remove_virtual_particles()");

    for (int i = 0; i < VirtualParticles.size(); i++)
    {
        if (VirtualParticles[i] != 0)
            VirtualParticles[i]->RemoveParticlesAtLevel(level);
        virtual_particles_set = false;
    }
}

void
Nyx::setup_ghost_particles(int ngrow)
{
    BL_PROFILE("Nyx::setup_ghost_particles()");
    BL_ASSERT(level < parent->finestLevel());

    if(Nyx::theDMPC() != 0)
    {
        DarkMatterParticleContainer::AoS ghosts;
        Nyx::theDMPC()->CreateGhostParticles(level, ngrow, ghosts);
        Nyx::theGhostPC()->AddParticlesAtLevel(ghosts, level+1, ngrow);
    }
#ifdef AGN
    if(Nyx::theAPC() != 0)
    {
        AGNParticleContainer::AoS ghosts;
        Nyx::theAPC()->CreateGhostParticles(level, ngrow, ghosts);
        Nyx::theGhostAPC()->AddParticlesAtLevel(ghosts, level+1, ngrow);
    }
#endif
#ifdef NEUTRINO_PARTICLES
    if(Nyx::theNPC() != 0)
    {
#ifdef NEUTRINO_DARK_PARTICLES
        DarkMatterParticleContainer::AoS ghosts;
#else
        NeutrinoParticleContainer::AoS ghosts;
#endif
        Nyx::theNPC()->CreateGhostParticles(level, ngrow, ghosts);
        Nyx::theGhostNPC()->AddParticlesAtLevel(ghosts, level+1, ngrow);
    }
#endif
    amrex::Gpu::Device::streamSynchronize();
}

void
Nyx::remove_ghost_particles()
{
    BL_PROFILE("Nyx::remove_ghost_particles()");

    for (int i = 0; i < GhostParticles.size(); i++)
    {
        if (GhostParticles[i] != 0)
            GhostParticles[i]->RemoveParticlesAtLevel(level);
    }
}

//NyxParticleContainerBase::~NyxParticleContainerBase() {}
