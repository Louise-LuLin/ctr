#include <stdio.h>
#include <string.h>
#include <string>
#include <getopt.h>
#include <vector>
#include <sstream>
#include "ctr.h"

using namespace std; 

gsl_rng * RANDOM_NUMBER = NULL;

void print_usage_and_exit() {
  // print usage information
  printf("*********************************collaborative topic models for recommendations************************\n");
  printf("Authors: Chong Wang, chongw@cs.princeton.edu, Computer Science Department, Princeton University.\n");
  printf("usage:\n");
  printf("      ctr [options]\n");
  printf("      --help:           print help information\n");

  printf("\n");
  printf("      --directory:      save directory, required\n");

  printf("\n");
  printf("      --user:           user file, required\n");
  printf("      --item:           item file, required\n");
  printf("      --a:              positive item weight, default 1\n");
  printf("      --b:              negative item weight, default 0.01 (b < a)\n");
  printf("      --lambda_u:       user vector regularizer, default 0.01\n");
  printf("      --lambda_v:       item vector regularizer, default 100\n");
  printf("      --learning_rate:  stochastic version for large datasets, default -1. Stochastic learning will be called when > 0.\n");
  printf("      --alpha_smooth:   alpha smooth, default [0.0]\n");
  printf("\n");

  printf("      --random_seed:    the random seed, default from the current time\n");
  printf("      --save_lag:       the saving lag, default 20 (-1 means no savings for intermediate results)\n");
  printf("      --max_iter:       the max number of iterations, default 200\n");
  printf("\n");

  printf("      --num_factors:    the number of factors, default 200\n");
  printf("      --mult:           mult file, in lda-c format, optional, if not provided, it's the matrix factorization\n");
  printf("      --:     topic proportions file from lda, optional (required if mult file is provided)\n");
  printf("      --beta_init:      topic distributions file from lda, optional (required if mult file is provided)\n");
  printf("      --theta_opt:      optimize theta or not, optional, default not\n");
  printf("      --lda_regression: run lda regression, default not\n");

  printf("*******************************************************************************************************\n");

  exit(0);
}

int main(int argc, char* argv[]) {
  // if (argc < 2) print_usage_and_exit();

  char filename[500];
  int theta_opt = 0;
  int lda_regression = 0;
  double a = 1.0;
  double b = 0.01;
  double lambda_u = 0.01;
  double lambda_v = 100;
  double learning_rate = -1;
  double alpha_smooth = 1.1;

  time_t t; time(&t);
  long   random_seed = (long) t;
  int    save_lag = 20;
  int    max_iter = 30;

  int    num_factors = 200;
  int    crossV = 1;

  string prefix="/zf18/ll5fy/lab/dataset";
  string source="YelpNew";
  string cold="true";

  string  directory = "";
  string  user_path = "";
  string  item_path = "";
  string  mult_path = "";
  string  theta_init_path = "";
  string  beta_init_path = "";

  int i=0;
  while (i <= argc - 1) {
    if (strcmp(argv[i], "-prefix") == 0) {
      prefix = string(argv[++i]);
      fprintf(stdout, "+ prefix = %s\n", prefix.c_str());
    } else if (strcmp(argv[i], "-source") == 0) {
      source = string(argv[++i]);
      fprintf(stdout, "+ source = %s\n", source.c_str());
    } else if (strcmp(argv[i], "-cold") == 0) {
      cold = string(argv[++i]);
      fprintf(stdout, "+ cold = %s\n", cold.c_str());
    } else if (strcmp(argv[i], "-iter") == 0){
      max_iter = atoi(argv[++i]);
      fprintf(stdout, "+ iter = %d\n", max_iter);
    } else if (strcmp(argv[i], "-crossV") == 0) {
      crossV = atoi(argv[++i]);
      fprintf(stdout, "+ crossV = %d\n", crossV);
    } else if (strcmp(argv[i], "-k") == 0) {
      num_factors = atoi(argv[++i]);
      fprintf(stdout, "+ k = %d\n", num_factors);
    } else if (strcmp(argv[i], "-theta_opt") == 0) {
      theta_opt = atoi(argv[++i]);
      fprintf(stdout, "+ theta_opt = %d\n", theta_opt);
    } else if (strcmp(argv[i], "-lr") == 0) {
      learning_rate = atof(argv[++i]);
      fprintf(stdout, "+ learning_rate = %d\n", learning_rate);
    } else if (i > 0) {
      fprintf(stdout,  "error: unknown option %s\n", argv[i]);
      assert(0);
    } 
    ++i;
  };

  std::vector<double*> perp;
  double* fold_perp;
  for (int cv_i = 0; cv_i < crossV; cv_i++) {
    /// print information
    printf("\n******************** Fold %d in %d, %s coldstart ******************\n", cv_i, crossV, cold.c_str());
    std::vector<std::string> test_path; 
    std::stringstream test;

    if (crossV == 1) {
      directory = prefix + "/output/" + source + "/byUser_20k_review";
      user_path = prefix + "/" + source + "/byUser_20k_review/CTR/user_false_0.txt";
      item_path = prefix + "/" + source + "/byUser_20k_review/CTR/item_false_0.txt";
      mult_path = prefix + "/" + source + "/byUser_20k_review/CTR/corpus_false_0.txt";
      theta_init_path = prefix + "/" + source + "/byUser_20k_review/CTR/" + std::to_string(num_factors) + ".doc.states";
      beta_init_path = prefix + "/" + source + "/byUser_20k_review/CTR/" + std::to_string(num_factors) + ".topics";
    } else {
      directory = "";
      user_path = prefix + "/" + source + "/byUser_20k_review/CTR/user_" + cold + "_" + std::to_string(cv_i) + ".txt";
      item_path = prefix + "/" + source + "/byUser_20k_review/CTR/item_" + cold + "_" + std::to_string(cv_i) + ".txt";
      mult_path = prefix + "/" + source + "/byUser_20k_review/CTR/corpus_" + cold + "_" + std::to_string(cv_i) + ".txt";
      theta_init_path = prefix + "/" + source + "/byUser_20k_review/CTR/" + cold + "_fold" + std::to_string(cv_i) + "_" + std::to_string(num_factors) + ".doc.states";
      beta_init_path = prefix + "/" + source + "/byUser_20k_review/CTR/" + cold + "_fold" + std::to_string(cv_i) + "_" + std::to_string(num_factors) + ".topics";

      if (strcmp(cold.c_str(), "true") == 0) {
        for (int j = 0; j < 3; j++) {
          test << prefix << "/" << source << "/byUser_20k_review/CTR/test_" << cold << "_" << cv_i << "_" << j << ".txt";
          test_path.push_back(test.str()); 
        }
      } else {
        test << prefix << "/" << source << "/byUser_20k_review/CTR/test_" << cold << "_" << cv_i << ".txt";
        test_path.push_back(test.str()); 
      }
    }
    
    if (crossV == 1) {
      if (!dir_exists(directory.c_str())) make_directory(directory.c_str());
      printf("result directory: %s\n", directory.c_str());
    }

    if (!file_exists(user_path.c_str())) {
      printf("user file %s doesn't exist! quit ...\n", user_path.c_str());
      exit(-1);
    }
    printf("user file: %s\n", user_path.c_str());

    if (!file_exists(item_path.c_str())) {
      printf("item file %s doesn't exist! quit ...\n", item_path.c_str());
      exit(-1);
    }
    printf("item file: %s\n", item_path.c_str());

    printf("a: %.4f\n", a);
    printf("b: %.4f\n", b);
    printf("lambda_u: %.4f\n", lambda_u);
    printf("lambda_v: %.4f\n", lambda_v);
    printf("learning_rate: %.5f\n", learning_rate);
    printf("alpha_smooth: %.5f\n", alpha_smooth);
    printf("random seed: %d\n", (int)random_seed);
    printf("save lag: %d\n", save_lag);
    printf("max iter: %d\n", max_iter);
    printf("number of factors: %d\n", num_factors);

    if (mult_path != "") {
      if (!file_exists(item_path.c_str())) {
        printf("mult file %s doesn't exist! quit ...\n", mult_path.c_str());
        exit(-1);
      }
      printf("mult file: %s\n", mult_path.c_str());
        
      if (theta_init_path == "") {
        printf("topic proportions file must be provided ...\n");
        exit(-1);
      }
      if (!file_exists(theta_init_path.c_str())) {
        printf("topic proportions file %s doesn't exist! quit ...\n", theta_init_path.c_str());
        exit(-1);
      }
      printf("topic proportions file: %s\n", theta_init_path.c_str());

      if (beta_init_path == "") {
        printf("topic distributions file must be provided ...\n");
        exit(-1);
      }
      if (!file_exists(beta_init_path.c_str())) {
        printf("topic distributions file %s doesn't exist! quit ...\n", beta_init_path.c_str());
        exit(-1);
      }
      printf("topic distributions file: %s\n", beta_init_path.c_str());
      if (theta_opt) printf("theta optimization: True\n");
      else printf("theta optimization: false\n");
    } else if (theta_opt) {
      printf("theta optimization: false");
      printf("(theta_opt has no effect, back to default value: false)\n");
      theta_opt = 0;
    }

    printf("\n");

    /// save the settings
    int ctr_run = 1;
    if (mult_path == "") ctr_run = 0;
    ctr_hyperparameter ctr_param;
    ctr_param.set(a, b, lambda_u, lambda_v, learning_rate, alpha_smooth,
        random_seed, max_iter, save_lag, theta_opt, ctr_run, lda_regression);
    // sprintf(filename, "%s/settings.txt", directory); 
    // ctr_param.save(filename);
    
    /// init random numbe generator
    RANDOM_NUMBER = new_random_number_generator(random_seed);

    // read users
    printf("reading user matrix from %s ...\n", user_path.c_str());
    c_data* users = new c_data(); 
    users->read_data(user_path.c_str());
    int num_users = (int)users->m_vec_data.size();

    // read items
    printf("reading item matrix from %s ...\n", item_path.c_str());
    c_data* items = new c_data(); 
    items->read_data(item_path.c_str());
    int num_items = (int)items->m_vec_data.size();

    // create model instance
    c_ctr* ctr = new c_ctr();
    ctr->set_model_parameters(num_factors, num_users, num_items);

    c_corpus* c = NULL;
    if (mult_path != "") {
      // read word data
      c = new c_corpus();
      c->read_data(mult_path.c_str());
      ctr->read_init_information(theta_init_path.c_str(), beta_init_path.c_str(), c, alpha_smooth);
    }

    vector<c_corpus*> test_c;
    for (int j = 0; j < test_path.size(); j++) {
      c_corpus* tmp_c = new c_corpus();
      tmp_c->read_data(test_path[j].c_str());
      test_c.push_back(tmp_c);
    }

    if (learning_rate <= 0) {
      fold_perp = ctr->learn_map_estimate(users, items, c, test_c, &ctr_param, directory.c_str());
      perp.push_back(fold_perp);
    } else {
      ctr->stochastic_learn_map_estimate(users, items, c, &ctr_param, directory.c_str());
    }

    free_random_number_generator(RANDOM_NUMBER);
    if (c != NULL) delete c;

    delete ctr;
    delete users;
    delete items;
    
  }

  int test_num = 1;
  if (strcmp(cold.c_str(), "true") == 0) {
    test_num = 3;
  }
  double mean = 0;
  double var = 0;
  for (int i = 0; i < test_num; i++) {
    mean = 0;
    var = 0;
    for (int j = 0; j < perp.size(); j++) {
      mean += *(perp[j]+i);
    }
    if(perp.size > 0)
      mean = mean/perp.size();

    for (int j = 0; j < perp.size(); j++) {
      var += (*(perp[j]+i) - mean) * (*(perp[j]+i) - mean);
    }
    if(perp.size > 0)
      var = sqrt(var/perp.size());

    printf("[Stat]Part %d Perplexity: %f+/-%f\n", i, mean, var);
  }

}
