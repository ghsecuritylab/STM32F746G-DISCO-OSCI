/**
  ******************************************************************************
  * @file         fft.h
  * @modified by  Werner Wolfrum
  * @date         03.03.2019
  * @modification Adaption for STM32F746G-DISCO using LL driver.
  *
  ******************************************************************************
  * Original file: name, date, version, author, source location
  * File     : fft.h
  * Datum    : ??.??.2014
  * Version  : ?.?
  * Autor    : UB
  * URL      : http://mikrocontroller.bplaced.net/wordpress/?page_id=752
  ******************************************************************************
  */

#ifndef __STM32F7_UB_FFT_H
#define __STM32F7_UB_FFT_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f7xx.h"
#include <math.h>
#include "arm_math.h"


//--------------------------------------------------------------
// FFT Buffer size
//--------------------------------------------------------------
#define FFT_LENGTH              512 // dont change
#define FFT_CMPLX_LENGTH        FFT_LENGTH*2
#define FFT_VISIBLE_LENGTH      FFT_LENGTH/2

//--------------------------------------------------------------
#define FFT_UINT_MAXWERT        100  // max = 100 Pixel


//--------------------------------------------------------------
// FFT Buffer
//--------------------------------------------------------------
float32_t FFT_DATA_IN[FFT_LENGTH];
uint16_t FFT_UINT_DATA[FFT_VISIBLE_LENGTH];


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
uint32_t fft_init(void);
void fft_calc(void);



//--------------------------------------------------------------
#endif // __STM32F7_UB_FFT_H
