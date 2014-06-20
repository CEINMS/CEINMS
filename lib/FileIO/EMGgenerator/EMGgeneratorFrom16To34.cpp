  // This is part of
  // NeuroMuscoloSkeletal Model Software (NMS)
  // Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
  //
  // ?? Licenza ??
  //
  // The authors may be contacted via:
  // email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#include <iostream>
using std::cout;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <stdlib.h>

#include "EMGgeneratorFrom16To34.h"


EMGgeneratorFrom16To34::EMGgeneratorFrom16To34(const std::string dummy){
  fromMusclesNames_.clear();
  
  fromMusclesNames_.push_back("addmag_r"); // 0															
  fromMusclesNames_.push_back("bicfemlh_r"); // 1
  fromMusclesNames_.push_back("gaslat_r"); // 2
  fromMusclesNames_.push_back("gasmed_r"); // 3
  fromMusclesNames_.push_back("gmax_r"); // 4
  fromMusclesNames_.push_back("gmed_r"); // 5
  fromMusclesNames_.push_back("gra_r"); // 6 
  fromMusclesNames_.push_back("perlong_r"); // 7
  fromMusclesNames_.push_back("recfem_r"); // 8
  fromMusclesNames_.push_back("sar_r"); // 9
  fromMusclesNames_.push_back("semimem_r"); // 10
  fromMusclesNames_.push_back("sol_r"); // 11
  fromMusclesNames_.push_back("tfl_r"); // 12
  fromMusclesNames_.push_back("tibant_r"); // 13
  fromMusclesNames_.push_back("vaslat_r"); // 14
  fromMusclesNames_.push_back("vasmed_r"); // 15

  
  if ( fromMusclesNames_.size() != noFromMuscles_ ) {  
    cout << "It's quite impossible that you are looking at this message!\n";
    exit (EXIT_FAILURE);
  }
  
  toMusclesNames_.clear();
  
  toMusclesNames_.push_back("addbrev_r");  //0
  toMusclesNames_.push_back("addlong_r");  //1
  toMusclesNames_.push_back("addmag1_r");  //2
  toMusclesNames_.push_back("addmag2_r");  //3
  toMusclesNames_.push_back("addmag3_r");  //4
  toMusclesNames_.push_back("bicfemlh_r");  //5
  toMusclesNames_.push_back("bicfemsh_r");  //6
  toMusclesNames_.push_back("gaslat_r");  //7
  toMusclesNames_.push_back("gasmed_r");  //8
  toMusclesNames_.push_back("gmax1_r");  //9
  toMusclesNames_.push_back("gmax2_r");  //10
  toMusclesNames_.push_back("gmax3_r");  //11
  toMusclesNames_.push_back("gmed1_r");  //12
  toMusclesNames_.push_back("gmed2_r");  //13
  toMusclesNames_.push_back("gmed3_r");  //14
  toMusclesNames_.push_back("gmin1_r");  //15
  toMusclesNames_.push_back("gmin2_r");  //16
  toMusclesNames_.push_back("gmin3_r");  //17
  toMusclesNames_.push_back("gra_r");  //18
  toMusclesNames_.push_back("illiacus_r");  //19
  toMusclesNames_.push_back("perbrev_r");  //20
  toMusclesNames_.push_back("perlong_r");  //21
  toMusclesNames_.push_back("perter_r");  //22
  toMusclesNames_.push_back("psoas_r");  //23
  toMusclesNames_.push_back("recfem_r");  //24
  toMusclesNames_.push_back("sar_r");  //25
  toMusclesNames_.push_back("semimem_r");  //26
  toMusclesNames_.push_back("semiten_r");  //27
  toMusclesNames_.push_back("sol_r");  //28
  toMusclesNames_.push_back("tfl_r");  //29
  toMusclesNames_.push_back("tibant_r");  //30
  toMusclesNames_.push_back("vasint_r");  //31
  toMusclesNames_.push_back("vaslat_r");  //32
  toMusclesNames_.push_back("vasmed_r");  //33

               
  if ( toMusclesNames_.size() != noToMuscles_ ) {  
    cout << "It's quite impossible that you are looking at this message!\n";
    exit (EXIT_FAILURE);
  }
  
}

void EMGgeneratorFrom16To34::convert(const vector<double>& fromEMG, vector<double>& toEMG) const {
  if (fromEMG.size() != noFromMuscles_) {
    cout << "I need " << noFromMuscles_ << " muscles, but I got " << fromEMG.size() << " muscles!\n";
    exit(EXIT_FAILURE); 
  }
  
  if (toEMG.size() != noToMuscles_) {
    cout << "I need " << noToMuscles_ << " muscles, but I got " << toEMG.size() << " muscles!\n";
    exit(EXIT_FAILURE); 
  }
  
  toEMG.clear();
  toEMG.resize(noToMuscles_);
  
  toEMG[0] = fromEMG[0]; // add_brev_r = add_mag_r  
  toEMG[1] = fromEMG[0]; // add_long_r = add_mag_r
  toEMG[2] = fromEMG[0]; // add_mag1_r = add_mag_r
  toEMG[3] = fromEMG[0]; // add_mag2_r = add_mag_r
  toEMG[4] = fromEMG[0]; // add_mag3_r = add_mag_r
  toEMG[5] = fromEMG[1]; // bifemlh_r = bifemlh_r
  toEMG[6] = fromEMG[1]; // bifemsh_r = bifemlh_r
  toEMG[7] = fromEMG[2]; // gaslat_r = gaslat_r
  toEMG[8] = fromEMG[3]; // gasmed_r = gasmed_r
  toEMG[9] = fromEMG[4]; // gmax1_r = gmax_r
  toEMG[10] = fromEMG[4]; // gmax2_r = gmax_r
  toEMG[11] = fromEMG[4]; // gmax3_r = gmax_r  
  toEMG[12] = fromEMG[5]; // gmed1_r= gmed_r
  toEMG[13] = fromEMG[5]; // gmed2_r= gmed_r
  toEMG[14] = fromEMG[5]; // gmed3_r= gmed_r
  toEMG[15] = (fromEMG[5] + fromEMG[4])/2; // gmin1_r= (gmed_r + gmax_r) /2
  toEMG[16] = (fromEMG[5] + fromEMG[4])/2; // gmin2_r= (gmed_r + gmax_r) /2
  toEMG[17] = (fromEMG[5] + fromEMG[4])/2; // gmin3_r= (gmed_r + gmax_r) /2
  toEMG[18] = fromEMG[6]; // gra_r = gra_r
  toEMG[19] = 0.;         // illiacus = 0
  toEMG[20] = fromEMG[7]; // perbrev_r = perlong_r
  toEMG[21] = fromEMG[7]; // perlong_r = perlong_r
  toEMG[22] = fromEMG[7]; // perter_r = perlong_r
  toEMG[23] = 0.;         // psoas = 0
  toEMG[24] = fromEMG[8]; // recfem_r = recfem_r
  toEMG[25] = fromEMG[9]; // sar_r = sar_r
  toEMG[26] = fromEMG[10]; // semimem_r = semimem_r
  toEMG[27] = fromEMG[10]; // semiten_r = semimem_r
  toEMG[28] = fromEMG[11]; // sol_r = sol_r 
  toEMG[29] = fromEMG[12]; // tfl_r = tfl_r
  toEMG[30] = fromEMG[13]; // tibant_r = tibant_r
  toEMG[31] = (fromEMG[14] + fromEMG[15])/2; // vasint_r = (vaslat_r +vasmed_r)/2
  toEMG[32] = fromEMG[14]; // vaslat_r = vaslat_r
  toEMG[33] = fromEMG[15]; // vasmed_r = vasmed_r

  }
