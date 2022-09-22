#!/bin/bash

query_features_dir=$1
reference_features_dir=$2
output_dir=$3
dataset_name=$4

#-----------------
cost_matrix_file=${output_dir}/${dataset_name}".CostMatrix.pb"
matching_result=${output_dir}/${dataset_name}".MatchingResult.pb"
output_result_image=${output_dir}/${dataset_name}"_result.png"
config_file=${output_dir}/${dataset_name}"_config.yaml"

# # Compute cost matrix.
# python compute_cost_matrix.py \
# --query_features ${query_features_dir}\
# --db_features ${reference_features_dir}\
# --cost_matrix_file ${cost_matrix_file}

# Create yaml config.

# # Run matching.
# ./src/apps/cost_matrix_based_matching/cost_matrix_based_matching_lsh  ${config_file}

# # Visualize matching results.
# python visualize_localization_result.py \
# --cost_matrix ${cost_matrix_file} \
# --matching_result ${matching_result}\
# --image_name ${output_result_image}