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

#include "EMGgeneratorFrom10To13.h"

EMGgeneratorFrom10To13::EMGgeneratorFrom10To13(){

  fromMusclesNames_.clear();
  fromMusclesNames_.push_back("semimem_r");
  fromMusclesNames_.push_back("bifemlh_r");
  fromMusclesNames_.push_back("sar_r");
  fromMusclesNames_.push_back("rect_fem_r");
  fromMusclesNames_.push_back("tfl_r");
  fromMusclesNames_.push_back("grac_r");
  fromMusclesNames_.push_back("vas_med_r");
  fromMusclesNames_.push_back("vas_lat_r");
  fromMusclesNames_.push_back("med_gas_r");
  fromMusclesNames_.push_back("lat_gas_r");
                                                                         
  if ( fromMusclesNames_.size() != noFromMuscles_ ) {  
    cout << "It's quite impossible that you are looking at this message!\n";
    exit (EXIT_FAILURE);
  }
  
  toMusclesNames_.clear();
  toMusclesNames_.push_back("semimem_r");
  toMusclesNames_.push_back("semiten_r");
  toMusclesNames_.push_back("bifemlh_r");
  toMusclesNames_.push_back("bifemsh_r");
  toMusclesNames_.push_back("sar_r");
  toMusclesNames_.push_back("rect_fem_r");
  toMusclesNames_.push_back("tfl_r");
  toMusclesNames_.push_back("grac_r");
  toMusclesNames_.push_back("vas_med_r");
  toMusclesNames_.push_back("vas_int_r");
  toMusclesNames_.push_back("vas_lat_r");
  toMusclesNames_.push_back("med_gas_r");
  toMusclesNames_.push_back("lat_gas_r");

  if ( toMusclesNames_.size() != noToMuscles_ ) {  
    cout << "It's quite impossible that you are looking at this message!\n";
    exit (EXIT_FAILURE);
  }
  
}

void EMGgeneratorFrom10To13::convert(const vector<double>& fromEMG, vector<double>& toEMG) const {
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
    
    toEMG[0] = fromEMG[0]; // semimem_r
    toEMG[1] = fromEMG[0]; // semiten_r = semimem_r
    toEMG[2] = fromEMG[1]; // bifemlh_r 
    toEMG[3] = fromEMG[1]; // bifemsh_r = bifemlh_r 
    toEMG[4] = fromEMG[2]; // sar_r 
    toEMG[5] = fromEMG[3]; // rect_fem_r
    toEMG[6] = fromEMG[4]; // tfl_r 
    toEMG[7] = fromEMG[5]; // grac_r 
    toEMG[8] = fromEMG[6]; // vas_med_r 
    toEMG[9] = ( fromEMG[6] + fromEMG[7] ) / 2; // vas_int_r = avg of vas_med_r and vas_lat_r 
    toEMG[10] = fromEMG[7]; // vas_lat_r 
    toEMG[11] = fromEMG[8]; // med_gas_r 
    toEMG[12] = fromEMG[9]; // lat_gas_r 
  
  }
