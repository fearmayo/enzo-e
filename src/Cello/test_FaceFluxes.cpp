// See LICENSE_CELLO file for license and copyright information

/// @file     test_FaceFluxes.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @date     2019-10-15
/// @brief    Test program for the FaceFluxes class

#include "main.hpp"
#include "test.hpp"

#include "data.hpp"

#include "test_setup_face.hpp"

double init_1(int ix, int iy, int iz, int mx, int my, int mz)
{
  return 7.0+3.0*(ix+mx*(iy+my*iz));
}

double init_2(int ix, int iy, int iz, int mx, int my, int mz)
{
  return 1.0+2.0*(ix+mx*(iy+my*iz));
}

PARALLEL_MAIN_BEGIN
{

  PARALLEL_INIT;

  unit_init(0,1);

  unit_class("FaceFluxes");

  const int num_face[2] = {4,6};
  const int face[][3] =
    {{+1,0,0},
     {-1,0,0},
     {0,+1,0},
     {0,-1,0},
     {0,0,+1},
     {0,0,-1}};

  const int num_normal=2;
  const int normal[] = {+1,-1};

  const int num_level = 3;
  const int level_1[] = {1,2,3};
  const int level_2[] = {1,3,2};

  int index_test = 0;

  auto test = test::face_test[index_test];

  for (int rank = 2; rank <=3; ++rank) {
    for (int iface=0; iface<num_face[rank-2]; ++iface) {
      for (int inormal=0; inormal<num_normal; ++inormal) {
        for (int ilevel=0; ilevel<num_level; ++ilevel) {
        
          const int L_1 = level_1[ilevel];
          const int L_2 = level_2[ilevel];
          int n3[3];
          n3[0] = test.size[0];
          n3[1] = (rank >= 2) ? test.size[1]:1;
          n3[2] = (rank >= 3) ? test.size[2]:1;
          int c3[3];
          c3[0] = test.centered[0];
          c3[1] = (rank >= 2) ? test.centered[1] : 0;
          c3[2] = (rank >= 3) ? test.centered[2] : 0;
          double h3_1[3];
          h3_1[0] = test.cell_width[0];
          h3_1[1] = (rank >= 2) ? test.cell_width[1] : 0;
          h3_1[2] = (rank >= 3) ? test.cell_width[2] : 0;
          double h3_2[3] = {h3_1[0],h3_1[1],h3_1[2]};
          const double dt1 =  test.time_step_1;
          double dt2 =  test.time_step_2;
          const int cx = test.child[0];
          const int cy = (rank >= 2) ? test.child[1] : 0;
          const int cz = (rank >= 3) ? test.child[2] : 0;

          if (L_1 < L_2) { h3_2[0]*=0.5; h3_2[1]*=0.5; h3_2[2]*=0.5; dt2*=0.5;}
          if (L_1 > L_2) { h3_2[0]*=2.0; h3_2[1]*=2.0; h3_2[2]*=2.0; dt2*=2.0;}
          int fx = face[iface][0];
          int fy = face[iface][1];
          int fz = face[iface][2];
          const int rx = normal[inormal]*fx;
          const int ry = normal[inormal]*fy;
          const int rz = normal[inormal]*fz;

          Face face_1(fx,fy,fz,rx,ry,rz);
          if (rx != 0) fx = -fx;
          if (ry != 0) fy = -fy;
          if (rz != 0) fz = -fz;
          Face face_2(fx,fy,fz,rx,ry,rz);
  
          int index_field = 3;
          auto face_fluxes_1 = new FaceFluxes
            (face_1,index_field, n3[0],n3[1],n3[2], L_1,dt1, c3[0],c3[1],c3[2]);
          auto face_fluxes_2 = new FaceFluxes
            (face_2,index_field, n3[0],n3[1],n3[2], L_2,dt2, c3[0],c3[1],c3[2]);

          unit_assert (face_fluxes_1 != NULL);
          unit_assert (face_fluxes_2 != NULL);

          unit_func ("get_dimensions()");
    
          int mx,my,mz;
          face_fluxes_1->get_dimensions (&mx,&my,&mz);

          unit_assert (mx == ((fx!=0) ? 1 : n3[0]));
          if (rank >= 2) unit_assert (my == ((fy!=0) ? 1 : n3[1]));
          if (rank >= 3) unit_assert (mz == ((fz!=0) ? 1 : n3[2]));

          int nx1,ny1,nz1;

          face_fluxes_1 -> get_size  (&nx1, &ny1, &nz1, rank, L_2);
    
          const int a0 = rx ? 0 : (ry ? 1 : 2);
    
          if (rank == 3) {
            // 3D
            int a1 = (a0+1) % 3;
            int a2 = (a0+2) % 3;
            unit_assert ( nx1*ny1*nz1 == n3[a1]*n3[a2]);
          } else if (rank == 2) {
            // 2D
            int a1 = (a0+1) % 2;
            unit_assert ( nx1*ny1*nz1 == n3[a1] );
          }

          //--------------------------------------------------

          int nx2,ny2,nz2;
          face_fluxes_2 -> get_size  (&nx2, &ny2, &nz2, rank, L_1);

          if (rank >= 3) {
            int a1 = (a0+1) % 3;
            int a2 = (a0+2) % 3;
            unit_assert ( nx2*ny2*nz2 == n3[a1]*n3[a2]);
          } else if (rank >= 2) {
            int a1 = (a0+1) % 2;
            unit_assert ( nx2*ny2*nz2 == n3[a1]);
          }

          face_fluxes_1->get_dimensions (&mx,&my,&mz);

          unit_assert (mx == ((fx!=0) ? 1 : n3[0]));
          unit_assert (my == ((fy!=0) ? 1 : (rank>=2)?n3[1] : 1));
          unit_assert (mz == ((fz!=0) ? 1 : (rank>=3)?n3[2] : 1));

          face_fluxes_1->get_dimensions (&mx,&my,&mz);

          unit_assert (mx == ((fx!=0) ? 1 : n3[0] + c3[0]));
          unit_assert (my == ((fy!=0) ? 1 : (rank>=2)?(n3[1] + c3[1]):1));
          unit_assert (mz == ((fz!=0) ? 1 : (rank>=3)?(n3[2] + c3[2]):1));


          unit_func ("allocate()");

          unit_assert(face_fluxes_1->flux_array().size() == 0);
          unit_assert(face_fluxes_2->flux_array().size() == 0);
          face_fluxes_1->allocate();
          face_fluxes_2->allocate();
          unit_assert(face_fluxes_1->flux_array().size() == mx*my*mz);
          unit_assert(face_fluxes_2->flux_array().size() == mx*my*mz);

          unit_func("set_flux_array()"); 

          std::vector<double> array_1;
          double sum_1 = 0;
          array_1.resize(mx*my*mz);
          for (int iz=0; iz<mz; iz++) {
            for (int iy=0; iy<my; iy++) {
              for (int ix=0; ix<mx; ix++) {
                int i=iz+mz*(iy+my*ix);
                array_1[i] = init_1(ix,iy,iz,mx,my,mz);
                sum_1 += array_1[i];
              }
            }
          }
          face_fluxes_1->set_flux_array(array_1,my*mz,mz,1);
    
          std::vector<double> array_2;
          double sum_2 = 0;
          array_2.resize(mx*my*mz);
          for (int iz=0; iz<mz; iz++) {
            for (int iy=0; iy<my; iy++) {
              for (int ix=0; ix<mx; ix++) {
                int i=iz+mz*(iy+my*ix);
                array_2[i] = init_2(ix,iy,iz,mx,my,mz);
                sum_2 += array_2[i];
              }
            }
          }
          face_fluxes_2->set_flux_array(array_2,my*mz,mz,1);


          int dx1,dy1,dz1;
          auto fluxes_1 = face_fluxes_1->flux_array(&dx1,&dy1,&dz1);
          int dx2,dy2,dz2;
          auto fluxes_2 = face_fluxes_2->flux_array(&dx2,&dy2,&dz2);
          unit_assert (fluxes_1.size() == mx*my*mz);
          unit_assert (fluxes_2.size() == mx*my*mz);

          unit_func ("flux_array()");
          bool match_1 = true, match_2=true;
          for (int iz=0; iz<mz; iz++) {
            for (int iy=0; iy<my; iy++) {
              for (int ix=0; ix<mx; ix++) {
                int i_f1=ix*dx1 + iy*dy1 + iz*dz1;
                int i_f2=ix*dx2 + iy*dy2 + iz*dz2;
                int i_a1=iz+mz*(iy+my*ix);
                int i_a2=iz+mz*(iy+my*ix);
                if ( fluxes_1[i_f1] != array_1[i_a1]) {
                  match_1 = false;
                }
                if ( fluxes_2[i_f2] != array_2[i_a2]) {
                  match_2 = false;
                }
              }
            }
          }
          unit_assert (match_1);
          unit_assert (match_2);

          // void set_centering(int c3[0], int c3[1], int c3[2])

          unit_func("face()");
          unit_assert (face_fluxes_1->face() == face_1);
          unit_assert (face_fluxes_2->face() == face_2);

          unit_func("level()"); 

          unit_assert (face_fluxes_1->level_neighbor() == L_1);
          unit_assert (face_fluxes_1->level_block()  == L_1);
          unit_assert (face_fluxes_2->level_neighbor() == L_2);
          unit_assert (face_fluxes_2->level_block()  == L_2);

          unit_func("time_step()"); 

          unit_assert (dt1 == face_fluxes_1->time_step_neighbor());
          unit_assert (dt2 == face_fluxes_2->time_step_neighbor());
          unit_assert (dt1 == face_fluxes_1->time_step_block());
          unit_assert (dt2 == face_fluxes_2->time_step_block());
          unit_assert (((L_1 == L_2) && (dt1 == dt2)) ||
                       ((L_1 != L_2) && (dt1 != dt2)));
          unit_assert (((L_1 == L_2) && (h3_1[0] == h3_2[0])) ||
                       ((L_1 != L_2) && (h3_1[0] != h3_2[0])));
          unit_assert (rank < 2 ||
                       (((L_1 == L_2) && (h3_1[1] == h3_2[1])) ||
                        ((L_1 != L_2) && (h3_1[1] != h3_2[1]))));
          unit_assert (rank < 3 ||
                       (((L_1 == L_2) && (h3_1[2] == h3_2[2])) ||
                        ((L_1 != L_2) && (h3_1[2] != h3_2[2]))));

          unit_func("float ratio_cell_volume()");

          double v_1 = (rank == 2) ? h3_1[0]*h3_1[1] : h3_1[0]*h3_1[1]*h3_1[2];
          double v_2 = (rank == 2) ? h3_2[0]*h3_2[1] : h3_2[0]*h3_2[1]*h3_2[2];
          float ratio = ratio_cell_volume(*face_fluxes_1,*face_fluxes_2,rank);
          unit_assert (ratio == v_1/v_2);
  
          unit_func("float ratio_time_step()");
  
          unit_assert (ratio_time_step(*face_fluxes_1,*face_fluxes_2) == dt1/dt2);

          // --------------------------------------------------
    
          //  void coarsen ()

          unit_func("coarsen()");   

          if (L_1 > L_2) {

            face_fluxes_1->coarsen(cx,cy,cz,rank);

            face_fluxes_1->get_dimensions (&mx,&my,&mz);

            int dxc,dyc,dzc;
            auto fluxes_coarse_1 = face_fluxes_1->flux_array(&dxc,&dyc,&dzc);
            double sum_1_coarse = 0;
            for (int iz=0; iz<mz; iz++) {
              for (int iy=0; iy<my; iy++) {
                for (int ix=0; ix<mx; ix++) {
                  int i=ix*dxc+iy*dyc+iz*dzc;
                  sum_1_coarse += fluxes_coarse_1[i];
                }
              }
            }

            unit_assert (sum_1 == sum_1_coarse);
      
      
          } else if (L_2 > L_1) {

            face_fluxes_2->get_dimensions (&mx,&my,&mz);

            face_fluxes_2->coarsen(cx,cy,cz,rank);

            face_fluxes_2->get_dimensions (&mx,&my,&mz);

            int dxc,dyc,dzc;
            auto fluxes_coarse_2 = face_fluxes_2->flux_array(&dxc,&dyc,&dzc);
            double sum_2_coarse = 0;
            for (int iz=0; iz<mz; iz++) {
              for (int iy=0; iy<my; iy++) {
                for (int ix=0; ix<mx; ix++) {
                  int i=ix*dxc+iy*dyc+iz*dzc;
                  sum_2_coarse += fluxes_coarse_2[i];
                }
              }
            }

            unit_assert (sum_2 == sum_2_coarse);
      
          }

          unit_assert (ratio_cell_volume(*face_fluxes_1,*face_fluxes_2,rank) == 1.0);

          unit_assert (face_fluxes_1->level_neighbor() ==
                       face_fluxes_2->level_neighbor());
          int L_min = std::min(L_1,L_2);
          unit_assert (face_fluxes_1->level_block()  == L_1);
          unit_assert (face_fluxes_1->level_neighbor() == L_min);
          unit_assert (face_fluxes_2->level_block()  == L_2);
          unit_assert (face_fluxes_2->level_neighbor() == L_min);

          //--------------------------------------------------

          //  FaceFluxes & operator *= (double)

          {
            unit_func("operator  *=()");

            auto & fluxes_1 = face_fluxes_1->flux_array(&dx1,&dy1,&dz1);
            int mx1,my1,mz1;
            face_fluxes_1->get_dimensions (&mx1,&my1,&mz1);
  
            double sum_1_pre = 0;
            for (int iz=0; iz<mz1; iz++) {
              for (int iy=0; iy<my1; iy++) {
                for (int ix=0; ix<mx1; ix++) {
                  int i=ix*dx1+iy*dy1+iz*dz1;
                  sum_1_pre += fluxes_1[i];
                }
              }
            }

            (*face_fluxes_1) *= (7.25);

            double sum_1_post = 0;
            for (int iz=0; iz<mz1; iz++) {
              for (int iy=0; iy<my1; iy++) {
                for (int ix=0; ix<mx1; ix++) {
                  int i=ix*dx1+iy*dy1+iz*dz1;
                  sum_1_post += fluxes_1[i];
                }
              }
            }

            unit_assert (sum_1_post == sum_1_pre * 7.25);
          }
    
          //--------------------------------------------------
    
          // FaceFluxes & operator += ()
    
          unit_func("operator += ()");

          {

            auto & fluxes_1 = face_fluxes_1->flux_array(&dx1,&dy1,&dz1);
            auto & fluxes_2 = face_fluxes_2->flux_array(&dx2,&dy2,&dz2);

            int mx1,my1,mz1;
            int mx2,my2,mz2;
            face_fluxes_1->get_dimensions (&mx1,&my1,&mz1);
            face_fluxes_2->get_dimensions (&mx2,&my2,&mz2);

            unit_func("operator +=");      

            double sum_1_pre = 0;
            for (int iz=0; iz<mz1; iz++) {
              for (int iy=0; iy<my1; iy++) {
                for (int ix=0; ix<mx1; ix++) {
                  int i=ix*dx1+iy*dy1+iz*dz1;
                  sum_1_pre += fluxes_1[i];
                }
              }
            }
            double sum_2_pre = 0;
            for (int iz=0; iz<mz2; iz++) {
              for (int iy=0; iy<my2; iy++) {
                for (int ix=0; ix<mx2; ix++) {
                  int i=ix*dx2+iy*dy2+iz*dz2;
                  sum_2_pre += fluxes_2[i];
                }
              }
            }

            double sum_1_post = 0;
            double sum_2_post = 0;

            if (L_1 <= L_2) {

              face_fluxes_1 -> accumulate(*face_fluxes_2,cx,cy,cz, rank);

              for (int iz=0; iz<mz1; iz++) {
                for (int iy=0; iy<my1; iy++) {
                  for (int ix=0; ix<mx1; ix++) {
                    int i=ix*dx1+iy*dy1+iz*dz1;
                    sum_1_post += fluxes_1[i];
                  }
                }
              }
            }
            if (L_1 >= L_2) {

              face_fluxes_2->accumulate(*face_fluxes_1,cx,cy,cz,rank);

              for (int iz=0; iz<mz2; iz++) {
                for (int iy=0; iy<my2; iy++) {
                  for (int ix=0; ix<mx2; ix++) {
                    int i=ix*dx2+iy*dy2+iz*dz2;
                    sum_2_post += fluxes_2[i];
                  }
                }
              }
            }

            if (L_1 < L_2) {
              unit_assert (sum_1_post == (sum_1_pre + sum_2_pre));
              unit_assert (sum_2_post == 0);
            } else if (L_1 > L_2) {
              unit_assert (sum_1_post == 0);
              unit_assert (sum_2_post == (sum_1_pre + sum_2_pre));
            } else if (L_1 == L_2) {
              unit_assert (sum_1_post == (sum_1_pre + sum_2_pre));
              unit_assert (sum_2_post == (sum_1_pre + 2*sum_2_pre));
            }
          }

          //--------------------------------------------------
          {
            unit_func("clear()");

            int mx1,my1,mz1;
            int mx2,my2,mz2;
            face_fluxes_1->get_dimensions (&mx1,&my1,&mz1);
            face_fluxes_2->get_dimensions (&mx2,&my2,&mz2);

            {
              double sum_abs = 0.0;
              auto & fluxes_1 = face_fluxes_1->flux_array(&dx1,&dy1,&dz1);
              for (int iz=0; iz<mz1; iz++) {
                for (int iy=0; iy<my1; iy++) {
                  for (int ix=0; ix<mx1; ix++) {
                    int i=ix*dx1+iy*dy1+iz*dz1;
                    sum_abs += std::abs(fluxes_1[i]);
                  }
                }
              }
              unit_assert(sum_abs != 0.0);
            }

            face_fluxes_1->clear();
      
            {
              double sum_abs = 0.0;
              auto & fluxes_1 = face_fluxes_1->flux_array(&dx1,&dy1,&dz1);
              for (int iz=0; iz<mz1; iz++) {
                for (int iy=0; iy<my1; iy++) {
                  for (int ix=0; ix<mx1; ix++) {
                    int i=ix*dx1+iy*dy1+iz*dz1;
                    sum_abs += std::abs(fluxes_1[i]);
                  }
                }
              }
              unit_assert(sum_abs == 0.0);
            }

            {
              double sum_abs = 0;
              auto & fluxes_2 = face_fluxes_2->flux_array(&dx2,&dy2,&dz2);
              for (int iz=0; iz<mz2; iz++) {
                for (int iy=0; iy<my2; iy++) {
                  for (int ix=0; ix<mx2; ix++) {
                    int i=ix*dx2+iy*dy2+iz*dz2;
                    sum_abs += std::abs(fluxes_2[i]);
                  }
                }
              }
              unit_assert(sum_abs != 0.0);
            }
            face_fluxes_2->clear();
            {
              double sum_abs = 0;
              auto & fluxes_2 = face_fluxes_2->flux_array(&dx2,&dy2,&dz2);
              for (int iz=0; iz<mz2; iz++) {
                for (int iy=0; iy<my2; iy++) {
                  for (int ix=0; ix<mx2; ix++) {
                    int i=ix*dx2+iy*dy2+iz*dz2;
                    sum_abs += std::abs(fluxes_2[i]);
                  }
                }
              }
              unit_assert(sum_abs == 0.0);
            }
          }
          // deallocate()
    
          unit_func("deallocate()");

          face_fluxes_1->deallocate();
          face_fluxes_2->deallocate();

          unit_assert (face_fluxes_1->flux_array().size() == 0);
          unit_assert (face_fluxes_2->flux_array().size() == 0);

          delete face_fluxes_1;
          delete face_fluxes_2;

          ++index_test ;
        } // ilevel
      } // inormal
    } // iface
  } // rank
  
  unit_finalize();

  exit_();
}

PARALLEL_MAIN_END

