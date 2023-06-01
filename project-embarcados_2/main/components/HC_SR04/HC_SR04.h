#ifndef HC_SR04_H
#define HC_SR04_H

void HC_SR04_init(int triggerPin, int echoPin);
float HC_SR04_readDistance();

#endif
