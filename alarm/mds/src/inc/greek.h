/*******************************************************************************
 * (C) COPYRIGHT Winway Systems Co., Ltd. 2014
 * All Rights Reserved
 * Licensed Materials - Property of WINWAY Co., Ltd.
 *
 * This program contains proprietary information of WINWAY Co., Ltd
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic, 
 * mechanical, or otherwise without the written permission of WINWAY.
 *
 *  Components   : greek.h - Marketdata processing System
 *  Release Ver  : 1.0.0
 ******************************************************************************/
#ifndef	_GREEK_H_
#define	_GREEK_H_
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mds.h"
#include "mdfold.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIE 	3.14159265358979323846
#define	max(x,y) x > y ? x:y

#define	ndOne(d1)	exp((pow(-d1, 2)) / 2) / (sqrt(2 * PIE))
#define	ndTwo(d2)	normsdist(d2);
#define	normdist(x)	exp((pow(-x, 2)) / 2) / (sqrt(2 * PIE))

// basic math functions for pricing
double fsum(const double *v, int n);
double favr(const double *v, int n);
double fround(double v, int nd);
double ftrunc(double v, int nd);
double normsdist(double x);
double stdev(const double *v, int n);
double correl(const double *x, const double *y, int n);
double randn(double mu, double sigma);

void   theBasis(double UnderlyingPrice, double MarketPrice, double TheoreticalPrice, double *mBasis, double *tBasis, double *DisparateRatio);
double bsOptionPrice(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
double bsMertonOptionPrice(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
double mcOptionPrice(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility, int nSimulations);
double bnOptionPrice(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility, int nSteps);
double bsDelta(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
double bsGamma(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
// double bsTheta(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
double bsTheta(int CallPut, double UnderlyingPrice, double StrikePrice, double eDays, double yDays, double Interest, double Dividend, double Volatility);
double bsRho(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
double bsVega(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);
double * bsMertonOptionsGreek(int CallPut, double UnderlyingPrice, double StrikePrice, double Time, double Interest, double Dividend, double Volatility);

#if 0
// volatility function
double hVolatility(MDHEOD *eod, int nArray, int nDays, double CalendarYear);
void   hVolatilities(double *hVol, MDHEOD *eod, int nArray, int nDays, double CalendarYear);
double iVolatility(int CallPut, double UnderlyingPrice, double StrikePrice, double CalendarYear, double Interest, double OptionPrice);
#endif


#ifdef __cplusplus
}
#endif

#endif
