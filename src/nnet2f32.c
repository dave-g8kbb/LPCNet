/*
  nnet2f32.c

  Writes current compiled-in model to a binary file of floats.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nnet_data.h"

void write_embedding_weights(char *name, const EmbeddingLayer *l, FILE *f32) {
    int n = l->nb_inputs*l->dim;
    printf("%s: %d\n", name, n);
    fwrite(l->embedding_weights, sizeof(float), n, f32);
}

void check_embedding_weights(char *name, const EmbeddingLayer *l, FILE *f32) {
    int n = l->nb_inputs*l->dim;
    printf("%s: %d", name, n);
    float *buf = malloc(sizeof(float)*n);
    assert(buf != NULL);
    int ret = fread(buf, sizeof(float), n, f32);    
    assert(ret == n);
    if (memcmp(l->embedding_weights, buf, n*sizeof(float)) == 0)
	printf(" OK\n");
    else {
	printf(" FAIL\n"); exit(1);
    }
    free(buf);
}

void check(const float *target, int n, FILE *f32) {
    float *buf = malloc(sizeof(float)*n); assert(buf != NULL);
    int ret = fread(buf, sizeof(float), n, f32);    
    assert(ret == n);
    if (memcmp(target, buf, n*sizeof(float)) == 0)
	printf(" OK");
    else {
	printf(" FAIL"); exit(1);
    }
    free(buf);
}

void check_int(const int *target, int n, FILE *f32) {
    int *buf = malloc(sizeof(int)*n); assert(buf != NULL);
    int ret = fread(buf, sizeof(int), n, f32);    
    assert(ret == n);
    if (memcmp(target, buf, n*sizeof(int)) == 0)
	printf(" OK");
    else {
	printf(" FAIL"); exit(1);
    }
    free(buf);
}

void write_dense_weights(char *name, const DenseLayer *l, FILE *f32) {
    int nbias = l->nb_neurons;
    int nweights = l->nb_inputs*l->nb_neurons;
    printf("%s: %d %d\n", name, nweights, nbias);
    fwrite(l->bias, sizeof(float), nbias, f32);
    fwrite(l->input_weights, sizeof(float), nweights, f32);
}

void check_dense_weights(char *name, const DenseLayer *l, FILE *f32) {
    int nbias = l->nb_neurons;
    int nweights = l->nb_inputs*l->nb_neurons;
    printf("%s: %d %d", name, nweights, nbias);
    check(l->bias, nbias, f32);
    check(l->input_weights, nweights, f32);
    printf("\n");
}

void write_mdense_weights(char *name, const MDenseLayer *l, FILE *f32) {
    int ninput = l->nb_inputs*l->nb_neurons*l->nb_channels;
    int nbias = l->nb_neurons*l->nb_channels;
    int nfactor = l->nb_neurons*l->nb_channels;
    printf("%s: %d %d %d\n", name, ninput, nbias, nfactor);
    fwrite(l->bias, sizeof(float), nbias, f32);
    fwrite(l->input_weights, sizeof(float), ninput, f32);
    fwrite(l->factor, sizeof(float), nfactor, f32);
}

void check_mdense_weights(char *name, const MDenseLayer *l, FILE *f32) {
    int ninput = l->nb_inputs*l->nb_neurons*l->nb_channels;
    int nbias = l->nb_neurons*l->nb_channels;
    int nfactor = l->nb_neurons*l->nb_channels;
    printf("%s: %d %d %d", name, ninput, nbias, nfactor);
    check(l->bias, nbias, f32);
    check(l->input_weights, ninput, f32);
    check(l->factor, nfactor, f32);
    printf("\n");
}

void write_conv1d_weights(char *name, const Conv1DLayer *l, FILE *f32) {
    int n = l->nb_inputs*l->kernel_size*l->nb_neurons;
    printf("%s: %d %d\n", name, n, l->nb_neurons);
    fwrite(l->input_weights, sizeof(float), n, f32);
    fwrite(l->bias, sizeof(float), l->nb_neurons, f32);
}

void check_conv1d_weights(char *name, const Conv1DLayer *l, FILE *f32) {
    int n = l->nb_inputs*l->kernel_size*l->nb_neurons;
    printf("%s: %d %d", name, n, l->nb_neurons);
    check(l->input_weights, n, f32);
    check(l->bias, l->nb_neurons, f32);
    printf("\n");
}

void write_gru_weights(char *name, const GRULayer *l, FILE *f32) {
    int nbias = l->nb_neurons*6;
    int ninput = l->nb_inputs*l->nb_neurons*3;
    int nrecurrent = l->nb_neurons*l->nb_neurons*3;
    printf("%s: %d %d %d\n", name, nbias, ninput, nrecurrent);
    fwrite(l->bias, sizeof(float), nbias, f32);
    fwrite(l->input_weights, sizeof(float), ninput, f32);
    fwrite(l->recurrent_weights, sizeof(float), nrecurrent, f32);
}

void check_gru_weights(char *name, const GRULayer *l, FILE *f32) {
    int nbias = l->nb_neurons*6;
    int ninput = l->nb_inputs*l->nb_neurons*3;
    int nrecurrent = l->nb_neurons*l->nb_neurons*3;
    printf("%s: %d %d %d", name, nbias, ninput, nrecurrent);
    check(l->bias, nbias, f32);
    check(l->input_weights, ninput, f32);
    check(l->recurrent_weights, nrecurrent, f32);
    printf("\n");
}

int sparse_sgemv_count_idx(int rows, const int *idx)
{
   int i, j;
   int count = 0;
   for (i=0;i<rows;i+=16) {
      int cols;
      cols = *idx++; count++;
      for (j=0;j<cols;j++) {
	  idx++; count++;
      }
   }
   return count; 
}

void write_sparse_gru_weights(char *name, const SparseGRULayer *l, FILE *f32) {
    int nbias = l->nb_neurons*6;
    int ndiag = l->nb_neurons*3;
    int nrecurrent = l->nb_neurons*l->nb_neurons*3;
    int nidx = sparse_sgemv_count_idx(ndiag, l->idx);
    printf("%s: %d %d %d %d\n", name, nbias, ndiag, nrecurrent, nidx);
    fwrite(l->bias, sizeof(float), nbias, f32);
    fwrite(l->diag_weights, sizeof(float), ndiag, f32);
    fwrite(l->recurrent_weights, sizeof(float), nrecurrent, f32);
    fwrite(l->idx, sizeof(int), nidx, f32);
}

void check_sparse_gru_weights(char *name, const SparseGRULayer *l, FILE *f32) {
    int nbias = l->nb_neurons*6;
    int ndiag = l->nb_neurons*3;
    int nrecurrent = l->nb_neurons*l->nb_neurons*3;
    int nidx = sparse_sgemv_count_idx(ndiag, l->idx);
    printf("%s: %d %d %d %d", name, nbias, ndiag, nrecurrent, nidx);
    check(l->bias, nbias, f32);
    check(l->diag_weights, ndiag, f32);
    check(l->recurrent_weights, nrecurrent, f32);
    check_int(l->idx, nidx, f32);
    printf("\n");
}

int main(int argc, char **argv) {

    if (argc != 2) {
	fprintf(stderr, "usage: %s model_file.f32\n", argv[0]);
	exit(0);
    }
    
    FILE *f32 = fopen(argv[1], "wb");
    assert(f32 != NULL);

    write_embedding_weights("gru_a_embed_sig.....", &gru_a_embed_sig, f32);
    write_embedding_weights("gru_a_embed_pred....", &gru_a_embed_pred, f32);
    write_embedding_weights("gru_a_embed_exc.....", &gru_a_embed_pred, f32);
    write_dense_weights    ("gru_a_dense_feature.", &gru_a_dense_feature, f32);
    write_embedding_weights("embed_pitch.........", &embed_pitch, f32);
    write_conv1d_weights   ("feature_conv1.......", &feature_conv1, f32);
    write_conv1d_weights   ("feature_conv2.......", &feature_conv2, f32);
    write_dense_weights    ("feature_dense1......", &feature_dense1, f32);
    write_embedding_weights("embed_sig...........", &embed_sig, f32);
    write_dense_weights    ("feature_dense2......", &feature_dense2, f32);
    write_gru_weights      ("gru_a...............", &gru_a, f32);
    write_gru_weights      ("gru_b...............", &gru_b, f32);
    write_mdense_weights   ("dual_fc.............", &dual_fc, f32);
    write_sparse_gru_weights("sparse_gru_a........", &sparse_gru_a, f32);
    fclose(f32);
    
    f32 = fopen(argv[1], "rb");
    check_embedding_weights("gru_a_embed_sig.....", &gru_a_embed_sig, f32);
    check_embedding_weights("gru_a_embed_pred....", &gru_a_embed_pred, f32);
    check_embedding_weights("gru_a_embed_exc.....", &gru_a_embed_pred, f32);
    check_dense_weights    ("gru_a_dense_feature.", &gru_a_dense_feature, f32);
    check_embedding_weights("embed_pitch.........", &embed_pitch, f32);
    check_conv1d_weights   ("feature_conv1.......", &feature_conv1, f32);
    check_conv1d_weights   ("feature_conv2.......", &feature_conv2, f32);
    check_dense_weights    ("feature_dense1......", &feature_dense1, f32);
    check_embedding_weights("embed_sig...........", &embed_sig, f32);
    check_dense_weights    ("feature_dense2......", &feature_dense2, f32);
    check_gru_weights      ("gru_a...............", &gru_a, f32);
    check_gru_weights      ("gru_b...............", &gru_b, f32);
    check_mdense_weights   ("dual_fc.............", &dual_fc, f32);
    check_sparse_gru_weights("sparse_gru_a........", &sparse_gru_a, f32);
  
    return 0;
}