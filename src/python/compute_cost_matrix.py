import numpy as np
from pathlib import Path
import argparse
import protos_io


def cosine_similarity(qu, db):
    norm = np.linalg.norm(qu) * np.linalg.norm(db)
    dist = np.dot(qu, db) / norm
    return dist


def compute_cost_matrix(qu_features, db_features):
    cost_matrix = np.zeros((len(qu_features), len(db_features)))

    print("Computing cost matrix...")
    for q, query_feature in enumerate(qu_features):
        for d, db_feature in enumerate(db_features):
            # Compute cosine distance
            cost_matrix[q, d] = cosine_similarity(query_feature, db_feature)

    print("Finished.")
    return cost_matrix


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--query_features", required=True, type=Path, help="Path to query directory."
    )
    parser.add_argument(
        "--db_features", required=True, type=Path, help="Path to db directory."
    )
    parser.add_argument(
        "--cost_matrix_file",
        required=False,
        default="cost_matrix.CostMatrix.pb",
        type=Path,
        help="File name for cost matrix output with extension CostMatrix.pb",
    )
    args = parser.parse_args()

    query_files = list(args.query_features.glob("*.Feature.pb"))
    db_files = list(args.db_features.glob("*.Feature.pb"))
    cost_matrix_file = args.cost_matrix_file

    query_files.sort()
    db_files.sort()

    query_features = []
    for query_file in query_files:
        query_features.append(protos_io.read_feature(query_file))

    db_features = []
    for db_file in db_files:
        db_features.append(protos_io.read_feature(db_file))

    cost_matrix = compute_cost_matrix(query_features, db_features)

    print("Matrix size", cost_matrix.shape)
    protos_io.write_cost_matrix(cost_matrix, cost_matrix_file)
    print("The cost matrix was save to", cost_matrix_file)


if __name__ == "__main__":
    main()
