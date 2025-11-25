#ifndef CAR_PROFILES_H
#define CAR_PROFILES_H

#include "car.h"

const CarProfile PORSCHE_911 = {
    .horsepower    = 520.0f,
    .mass          = 1420.0f,
    .dragCoeff     = 0.03f,
    .rollingCoeff  = 1.0f,
    .tireMu        = 1.3f,
    .weightDistrib = 0.62f,
    .turnRadius    = 155.0f,
    .frontAero     = 0.16f,
    .rearAero      = 0.28f,
    .brake         = 26000.0f
};

const CarProfile HONDA_NSX = {
    .horsepower    = 390.0f,
    .mass          = 1210.0f,
    .dragCoeff     = 0.03f,
    .rollingCoeff  = 1.0f,
    .tireMu        = 1.5f,
    .weightDistrib = 0.58f,
    .turnRadius    = 152.0f,
    .frontAero     = 0.18f,
    .rearAero      = 0.25f,
    .brake         = 23000.0f
};

const CarProfile LAMBORGHINI_GALLARDO = {
    .horsepower    = 550.0f,
    .mass          = 1590.0f,
    .dragCoeff     = 0.031f,
    .rollingCoeff  = 1.0f,
    .tireMu        = 1.22f,
    .weightDistrib = 0.57f,
    .turnRadius    = 148.0f,
    .frontAero     = 0.17f,
    .rearAero      = 0.30f,
    .brake         = 24000.0f
};

const CarProfile FORD_GT = {
    .horsepower    = 550.0f,
    .mass          = 1530.0f,
    .dragCoeff     = 0.029f,
    .rollingCoeff  = 1.0f,
    .tireMu        = 1.00f,
    .weightDistrib = 0.56f,
    .turnRadius    = 160.0f,
    .frontAero     = 0.19f,
    .rearAero      = 0.33f,
    .brake         = 22500.0f
};

#endif
