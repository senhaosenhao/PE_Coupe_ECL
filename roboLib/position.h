#ifndef POSITION_H
#define POSITION_H

#include "config.h"

//Structure de donn�es de position.
//Distances en mm, angles en degr�s.
struct PosData {
	float x;
	float y;
	float orientation;
}

void initPosition(bool isGreenSide);

//Renvoie un posData constenant les donn�es de position du robot, en mm et degr�s.
void getPosition(struct PosData const** data);

//M�me chose en ticks d'encode et radians, plus rapide.
void getRawPosition(struct PosData const** data);


//-- DEBUT DES DEFINITIONS --


PosData __position; //Contient en tout temps la position du robot (r�guli�rement mise � jour) en tics d'encodage.

PosData __pos_standardized; //Contient la position convertie en mm, ne pas utiliser ; mis � jour dans getPosition() uniquement.

float __oldEncoderL;
float __oldEncoderR;
float __betweenWheelsInEncode;

void getPosition(struct PosData const** data) {
	struct Config const* c = NULL;
	getConfig(&c);
  __pos_standardized.x = __position.x * c->mmPerEncode;
  __pos_standardized.y = __position.y * c->mmPerEncode;
  __pos_standardized.orientation = __position.orientation * 180 / PI;
  *data = &__pos_standardized;
}

void getRawPosition(struct PosData const** data) {
	*data = &__position;
}

task updatePosition() {
  while (true) {
    wait1Msec(20);
    float deltaR = nMotorEncoder[motorA] - __oldEncoderR;
    __oldEncoderR = nMotorEncoder[motorA];
    float deltaL = nMotorEncoder[motorB] - __oldEncoderL;
    __oldEncoderL = nMotorEncoder[motorB];
    float deltaD = (deltaR + deltaL) / 2;
    float deltaO = (deltaR - deltaL) / __betweenWheelsInEncode;
    float midO = __position.orientation + deltaO / 2;
    __position.x += cos(midO) * deltaD;
    __position.y += sin(midO) * deltaD;
    __position.orientation += deltaO;
    if (__position.orientation < 0)
    	__position.orientation += 2*PI;
  	else if (__position.orientation > 2*PI)
  		__position.orientation -= 2*PI;
  }
}

void initPosition(bool isGreenSide) {
	struct Config const* c = NULL;
	getConfig(&c);

  if (isGreenSide) {
    __position.x = c->initialX_GreenSide;
    __position.y = c->initialY_GreenSide;
    __position.orientation = c->initialOrientation_GreenSide;
  }
  else {
    __position.x = c->initialX_OrangeSide;
    __position.y = c->initialY_OrangeSide;
    __position.orientation = c->initialOrientation_OrangeSide;
  }
  nMotorEncoder[motorB] = 0;
  nMotorEncoder[motorA] = 0;
  __oldEncoderL = 0;
  __oldEncoderR = 0;
  __betweenWheelsInEncode = c->betweenWheels / c->mmPerEncode;
  startTask(updatePosition);
}

#endif
