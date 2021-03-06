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
#include <math.h> //log2
#include "operators.h"
#include "structs_ga.h"
#include "calc_fitness.h"
#include "sort_population.h"
#include "entropy.h"

/* NOTE: The parameters are the same proposed in the paper:
 [Memetic Algorithm for Sorting Unsigned Permutations by Reversals] */


/***Parameters for MA***/
const float crossover_prob      = 0.98;     //Crossover Probability
const float mutation_prob       = 0.01;     //Mutation Probability
const float selection_pct       = 0.96;     //Percentage(pct) of the pop. that will be selected for Crossover
const float replacement_pct     = 1 - 0.6;  //Pct. of the population from where it will be performed
                                              //the Replacement of the offspring 
                                              //(1-0.6=0.4 means that 60% (0.6) of the population at the end will be replaced)

const float localsearch_pct     = 0.94;     //Pct. of the population that will be selected for Local Search
const float conservation_pct    = 0.98;     //Pct. of Preservation of the population when it is restarted
const float restart_param       = 0.2;      //Parameter for Restarting the population



/* Procedure that generates the initial population
 */
void generate_initial_population(population *pob, permutation *perm){
	int i,j,r,n,prob;
	
    //calculate length of population
	n = perm->length;
	pob->population_size =  (int) (n * (log(n) / log(2))); //length of population = (nlogn); log in base 2

	pob->population_base = 0;
	pob->population_top = pob->population_size;
    
	pob->limit_for_selection = pob->population_size * selection_pct;//limit from where individuals will be selected for crossover
	pob->replacement_base  = pob->population_size * replacement_pct;//base from where the replacement will be performed
    
	pob->offspring_base = pob->population_size;
	pob->offspring_top = pob->population_size + pob->limit_for_selection;
	pob->total_size = pob->offspring_top;
    
    /*allocate memory*/
	pob->chromosomes = malloc(pob->total_size*sizeof(chromosome)); //allocate memory for population
    if (pob->chromosomes == NULL) 
        printf("pob->chromosomes NULL en poblacion_inicial() de operadores.c\n");
    for(i=0;i<pob->total_size;i++){
        pob->chromosomes[i].genes = malloc(n * sizeof(int));//allocate memory for each chromosome
        if (pob->chromosomes[i].genes == NULL) 
            printf("pob->chromosomes[i].genes NULL en poblacion_inicial() de operadores.c\n");
    }
    
	/*create population from permutation*/
	for(i=pob->population_base; i < pob->population_top; i++){
		for(j = 0; j < perm->length; j++){
            r=rand(); 
			prob = r % 2;//values between 0 and 1
			if (prob == 1)//if prob 1, then positive gen
				pob->chromosomes[i].genes[j] = perm->pi[j];
			else//if prob 0, then negative gen
				pob->chromosomes[i].genes[j] = perm->pi[j] * -1;
		}
        
		pob->chromosomes[i].length = perm->length;
        
	}
	/*add length to individuals of ofsspring*/
	for(i=pob->offspring_base; i<pob->offspring_top; i++){
		pob->chromosomes[i].length = perm->length;
	}
}


/* This method restart the population, preserving a part of it.
 * The restarted population is improved by local search.
 */
void restart_population_LS(population *pob, permutation *perm, int *num_eval_fit_function){
	int i,j, prob, conservation_size;

	/* calculate conservation size of population */
	conservation_size = pob->population_size * conservation_pct;

	/* generate new population improved by local search */
	for(i=conservation_size; i < pob->population_size; i++){
		
        /* generate chromosome "i" randomly */
		for(j = 0; j < perm->length; j++){
			prob = rand() % 2;//values between 0 and 1
			if (prob == 1)
				pob->chromosomes[i].genes[j] = perm->pi[j];
			else
				pob->chromosomes[i].genes[j] = perm->pi[j] * -1;
		}

		pob->chromosomes[i].fitness = calculate_fitness(pob->chromosomes[i],num_eval_fit_function);

		/* apply local search over chromosome "i" */
		if (local_search(pob, i,num_eval_fit_function)){
			//printf("\n");
		}
	}

}

/* Calculate fitness of individuals of population
 * using the linear algorithm for the reversal distance
 * for signed permutations */
void calculate_fitness_population(population *pob, int generation_number, int *num_eval_fit_function){

	int i;
	if (generation_number > 1){ //fitness over offspring
		for(i=pob->offspring_base; i < pob->offspring_top; i++){
			pob->chromosomes[i].fitness = calculate_fitness(pob->chromosomes[i],num_eval_fit_function);
		}
	}
	else{ //generation_number == 1, fitness over population
		for(i=pob->population_base; i < pob->population_top; i++)
			pob->chromosomes[i].fitness = calculate_fitness(pob->chromosomes[i],num_eval_fit_function);
	}
}

void selection(population *pob,  int generation_number, int *best_solution){ 
	
	/* sort population using counting sort */
	//quickSort(pob,pob->population_base,pob->population_top-1);
    countingSort(pob,pob->chromosomes[0].length+2);

	/* update best solution */
    if (pob->chromosomes[0].fitness < *best_solution)
        *best_solution = pob->chromosomes[0].fitness;

}

void crossover(population *pob, permutation *perm){
	int i,j,k,pos_parent1, pos_parent2, crossover_point;
	float prob;

	k = pob->offspring_base;
	/* crossover over current pop. until limit for selection */
	for(i=pob->population_base; i<pob->limit_for_selection; i+=2){
		//if there is not space for offspring, finish
		if (k+1 == pob->total_size)
			break;

		//choose positions for parents
		pos_parent1 = rand() % pob->limit_for_selection;//values between 0 and limit_for_selection-1
		pos_parent2 = rand() % pob->limit_for_selection;//values between 0 and limit_for_selection-1
		//verify that pos for parent1 is not the same for parent2
		for(j=0; j<5; j++){
			if (pos_parent1 != pos_parent2) break;
			pos_parent2 = rand() % pob->limit_for_selection;//values between 0 and limit_for_selection-1
		}

		prob = ((rand() % 100) + 1)/100.0; //de 0.01, 0.02 ... a 1.0
		//if prob <= croosover_prob, apply the crossover
		if (prob <= crossover_prob){
			crossover_point = rand() % perm->length;//values between 0 and perm->length-1
			//apply single_point_crossover over the first part before the crossover point
			for(j=0; j<crossover_point; j++){
				pob->chromosomes[k].genes[j] = pob->chromosomes[pos_parent1].genes[j];
				pob->chromosomes[k+1].genes[j] = pob->chromosomes[pos_parent2].genes[j];
			}
			//apply single_point_crossover over the second part after the crossover point
			for(j=crossover_point; j<perm->length; j++){
				pob->chromosomes[k+1].genes[j] = pob->chromosomes[pos_parent1].genes[j];
				pob->chromosomes[k].genes[j] = pob->chromosomes[pos_parent2].genes[j];
			}
			k = k + 2;
		}
	}
	pob->offspring_top = k;//update offspring top
}

void mutation(population *pob, permutation *perm){
	int i,j;
	float prob;
	/*mutacion sobre descendencia*/
	for(i=pob->offspring_base; i<pob->offspring_top; i++){
		for(j=0; j<perm->length; j++){
			prob = ((rand() % 100) + 1)/100.0; //de 0.01, 0.02 ... a 1.0
			if(prob <= mutation_prob){//mutar gen (invertir signo)
				pob->chromosomes[i].genes[j] = pob->chromosomes[i].genes[j] * -1;
			}

		}
	}

}

void replacement(population *pob){
	int i,j,replacement_pos;
	
	/* replace offspring into the current population */
	for(i=pob->offspring_base; i<pob->offspring_top; i++){
		//calculate replacement position 
		int replacement_size = pob->population_top - pob->replacement_base;
		replacement_pos = 
					(rand() % replacement_size) + 
					pob->replacement_base;//(values betwen 0 and replacemet_size-1)+base
		
        //perform replacement
		if (pob->chromosomes[i].fitness < pob->chromosomes[replacement_pos].fitness){
			//copy the fitness value
            pob->chromosomes[replacement_pos].fitness = pob->chromosomes[i].fitness;
            //copy the genes
            for(j=0; j<pob->chromosomes[0].length; j++)
                pob->chromosomes[replacement_pos].genes[j] = pob->chromosomes[i].genes[j];
            
        }
	}
}

/* Procedure that applies Local Search over the population
 * if generation_number is equal to 1, it means is the initial pop.
 */
void LS_for_population(population *pob, int generation_number,int *num_eval_fit_function){  

	int i;
	if (generation_number > 1){
		/* limit to where it will be applied local search */
        int limit_for_local_search = pob->population_size * localsearch_pct;
        
		/* local search over a part of the population */
        for(i=pob->population_base; i < limit_for_local_search; i++){
            if (local_search(pob, i,num_eval_fit_function)){//local search over chromosome i
                //cont++;
            }
        }
    }
    else{ //generation_number == 1, is the initial population
        for(i=pob->population_base; i < pob->population_top; i++){
            if (local_search(pob, i,num_eval_fit_function)){
                //printf("contar -->> \n");
            } 
        }
    }

}

/* Local search operator based in mutation */
int local_search(population *pob, int i, int *num_eval_fit_function){
	int k, r, replace, fitness, number_iterations, pos_gen, best_fitness;
	
	number_iterations = 2;    
	k=1;
	replace = 0;//false
	best_fitness = pob->chromosomes[i].fitness;
	while(1){
		/* generate a random gen position */
        r=rand(); 
		pos_gen = r % pob->chromosomes[0].length;//values between 0 and length-1 
        
		/* modify gen in "pos_gen" */
		pob->chromosomes[i].genes[pos_gen] = pob->chromosomes[i].genes[pos_gen]*(-1);
		fitness = calculate_fitness(pob->chromosomes[i],num_eval_fit_function);
        
		if (fitness < best_fitness){
			pob->chromosomes[i].fitness = fitness;
			replace = 1;//true

			break;//break while
		}
		else{
			//go back to last state of gen "j"
			pob->chromosomes[i].genes[pos_gen] =
            pob->chromosomes[i].genes[pos_gen]*(-1);
		}
        
		if (k >= number_iterations) break;//break while
		else k++;
	}
    
	return replace;
}

int ma_reached_degenerate_state(population *pob){
    if (shannon_entropy(pob)  < restart_param)
        return TRUE;
    else
        return FALSE;

}   


/*****ADDITIONAL PROCEDURES******/

void read_permutation(permutation *perm){
    int i,n;
    scanf("%d",&n);//read length of permutation
    perm->length = n;
    //allocate memory for perm->pi
    perm->pi = malloc(perm->length*sizeof(int));
    for(i=0; i<n; i++){
        scanf("%d",&perm->pi[i]);
    }
}

void read_permutation_from_file(char* nombre_archivo, permutation *perm){
    int i,n;
    FILE *filePointer;
    if ((filePointer = fopen(nombre_archivo,"r")) == NULL){
        printf("Input file could not be opened\n");
    }
    else{
        //read length of permutation
        fscanf(filePointer,"%d",&n);
        perm->length = n;
        //allocate memory for perm->pi
        perm->pi = malloc(perm->length*sizeof(int));
        //leer permutation
        for(i=0; i<n; i++){
            fscanf(filePointer,"%d",&perm->pi[i]);
        }
    }
    fclose(filePointer);
}

void show_population(population *pop, permutation *perm, int generation_number){
    
    int i,j;
    printf("*** Generation %d ***\n", generation_number);
    //print current population
    for(i=pop->population_base; i<pop->population_top; i++){
        printf("{");
        for(j=0; j<perm->length-1; j++){
            printf("%d, ", pop->chromosomes[i].genes[j]);
        }
        printf("%d", pop->chromosomes[i].genes[perm->length-1]);
        printf("} fit: %d\n",pop->chromosomes[i].fitness);
    }
    //print offspring
    printf("-------------------\n");
    for(i=pop->offspring_base; i<pop->offspring_top; i++){
        printf("{");
        for(j=0; j<perm->length-1; j++){
            printf("%d, ", pop->chromosomes[i].genes[j]);
        }
        printf("%d", pop->chromosomes[i].genes[perm->length-1]);
        printf("} fit: %d\n",pop->chromosomes[i].fitness);
    }
    
}







