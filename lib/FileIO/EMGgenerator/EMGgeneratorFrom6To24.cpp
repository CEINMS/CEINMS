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

#include "EMGgeneratorFrom6To24.h"


EMGgeneratorFrom6To24::EMGgeneratorFrom6To24(){  
  fromMusclesNames_.clear();
  
  fromMusclesNames_.push_back("bicfemlh_r"); // 0
  fromMusclesNames_.push_back("gmax_r"); // 1
  fromMusclesNames_.push_back("gmed_r"); // 2
  fromMusclesNames_.push_back("recfem_r"); // 3
  fromMusclesNames_.push_back("semiten_r"); // 4
  fromMusclesNames_.push_back("tfl_r"); // 5

  
  if ( fromMusclesNames_.size() != noFromMuscles_ ) {  
    cout << "It's quite impossible that you are looking at this message!\n";
    exit (EXIT_FAILURE);
  }
  
  toMusclesNames_.clear();
  
  toMusclesNames_.push_back("addbrev_r");   //0
  toMusclesNames_.push_back("addlong_r");   //1
  toMusclesNames_.push_back("addmag1_r");   //2
  toMusclesNames_.push_back("addmag2_r");   //3
  toMusclesNames_.push_back("addmag3_r");   //4
  toMusclesNames_.push_back("bicfemlh_r");  //5
  toMusclesNames_.push_back("bicfemsh_r");  //6
  toMusclesNames_.push_back("gmax1_r");     //7
  toMusclesNames_.push_back("gmax2_r");     //8
  toMusclesNames_.push_back("gmax3_r");     //9
  toMusclesNames_.push_back("gmed1_r");     //10
  toMusclesNames_.push_back("gmed2_r");     //11
  toMusclesNames_.push_back("gmed3_r");     //12
  toMusclesNames_.push_back("gmin1_r");     //13
  toMusclesNames_.push_back("gmin2_r");     //14
  toMusclesNames_.push_back("gmin3_r");     //15
  toMusclesNames_.push_back("gra_r");       //16
  toMusclesNames_.push_back("illiacus_r");  //17
  toMusclesNames_.push_back("psoas_r");     //18
  toMusclesNames_.push_back("recfem_r");    //19
  toMusclesNames_.push_back("sar_r");       //20
  toMusclesNames_.push_back("semimem_r");   //21
  toMusclesNames_.push_back("semiten_r");   //22
  toMusclesNames_.push_back("tfl_r");       //23
  
               
  if ( toMusclesNames_.size() != noToMuscles_ ) {  
    cout << "It's quite impossible that you are looking at this message!\n";
    exit (EXIT_FAILURE);
  }
  
}

void EMGgeneratorFrom6To24::convert(const vector<double>& fromEMG, vector<double>& toEMG) const {
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
  
  toEMG[0] = 0.;         // addbrev_r = 0
  toEMG[1] = 0.;         // addlong_r = 0
  toEMG[2] = 0.;         // addmag1_r = 0
  toEMG[3] = 0.;         // addmag2_r = 0
  toEMG[4] = 0.;         // addmag3_r = 0
  toEMG[5] = fromEMG[0]; // bifemlh_r = bifemlh_r
  toEMG[6] = fromEMG[0]; // bifemsh_r = bifemlh_r
  toEMG[7] = fromEMG[1]; // gmax1_r = gmax_r
  toEMG[8] = fromEMG[1]; // gmax2_r = gmax_r
  toEMG[9] = fromEMG[1]; // gmax3_r = gmax_r  
  toEMG[10] = fromEMG[2]; // gmed1_r= gmed_r
  toEMG[11] = fromEMG[2]; // gmed2_r= gmed_r
  toEMG[12] = fromEMG[2]; // gmed3_r= gmed_r
  toEMG[13] = (fromEMG[2] + fromEMG[1])/2; // gmin1_r= (gmed_r + gmax_r) /2
  toEMG[14] = (fromEMG[2] + fromEMG[1])/2; // gmin2_r= (gmed_r + gmax_r) /2
  toEMG[15] = (fromEMG[2] + fromEMG[1])/2; // gmin3_r= (gmed_r + gmax_r) /2
  toEMG[16] = 0.;         // gra_r = 0
  toEMG[17] = 0.;         // illiacus = 0
  toEMG[18] = 0.;         // psoas = 0
  toEMG[19] = fromEMG[3]; // recfem_r = recfem_r
  toEMG[20] = 0.;         // sar_r = 0
  toEMG[21] = fromEMG[4]; // semimem_r = semiten_r
  toEMG[22] = fromEMG[4]; // semiten_r = semiten_r
  toEMG[23] = fromEMG[5]; // tfl_r = tfl_r
 
  
  
  }
