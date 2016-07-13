/*
 ============================================================================
 Author :
 Jose Luis Soncco-Alvarez and Mauricio Ayala-Rincon
 Group of Theory of Computation
 Universidade de Brasilia (UnB) - Brazil
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h> //malloc, free
#include <string.h> //strcpy
#include <math.h>   //log2
#include "uthash.h"
#include "structs_ga.h"
#include "calc_fitness.h"
#include "entropy.h"

struct mi_struct {
	int             id;				/* index used in the hashtable (fitness) */
	int             num_cases;      /* number of ocurrences of chromosome id */
	double          probability;    /* probability of chromosome id */
	UT_hash_handle  hh;             /* make this structure a "hashable */
};

struct mi_struct *elements = NULL;

void add_element(int id_elto){
	struct mi_struct *s;

	HASH_FIND_INT(elements, &id_elto, s);  /* id already in the hash? */
	if (s==NULL) {
		/* add new element */
		s = (struct mi_struct*)malloc(sizeof(struct mi_struct));
		s->id = id_elto;
		s->num_cases = 1;
		s->probability = 0;
		HASH_ADD_INT( elements, id, s );  /* id: name of key field */
	}
	else { /* s already exists */
		/* increment num of cases */
		s->num_cases++;
	}
}

void eliminate_elements() {
	struct mi_struct *elto_actual, *tmp;

	HASH_ITER(hh, elements, elto_actual, tmp) {
		HASH_DEL(elements, elto_actual); /* delete it (users advances to next) */
		free(elto_actual); /* free it */
	}
}

void show_elements() {
    struct mi_struct *s;

    for(s=elements; s != NULL; s=(struct mi_struct*)(s->hh.next)) {
        printf("user id %d: num cases %d\n", s->id, s->num_cases);
    }
}


/* Method for calculating the shannon entropy */
float shannon_entropy(population *pob) {
	int i, /*j,*/ id, num_eltos;
	struct mi_struct *s;
	float prob, logb2, h, max_h, d;

	/* create hashtable for population */
	for(i=0; i < pob->population_size; i++) {
		id = 0;

		/* Adicionar id */
		id = pob->chromosomes[i].fitness;
		add_element(id);

	}

	/* calculate shannon entropy */
	h=0;
	num_eltos = HASH_COUNT(elements);
	for(s=elements; s != NULL; s=(struct mi_struct*)(s->hh.next)) {
		prob = s->num_cases * 1.0 / pob->population_size; //defition of probability
		logb2 = (log(prob) / log(2)); //property of logarithms (base change)
		h = h + prob*logb2; // sum prob*log(prob)b2
		//printf("prob: %f, logb2: %f, m:%f\n",prob, logb2, prob*logb2);
	}
	h = -1*h; //h: shannon entropy
	max_h = log(num_eltos) / log(2); //max shannon entropy log(num_var)b2
	d = h / max_h; // relative entropy

	/* eliminate hash table */
	eliminate_elements();

	return d;

}










