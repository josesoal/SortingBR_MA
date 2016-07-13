/*
 ============================================================================
 Author :
 Jose Luis Soncco-Alvarez and Mauricio Ayala-Rincon
 Group of Theory of Computation
 Universidade de Brasilia (UnB) - Brazil
 ============================================================================
 */

#ifndef ENTROPY_H_
#define ENTROPY_H_

#include "structs_ga.h"

void add_element(int id_elto);
void eliminate_elements();
void show_elements();
float shannon_entropy(population *pob);

#endif /* ENTROPIA_H_ */
