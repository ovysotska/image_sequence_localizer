import numpy as np
from pathlib import Path
import argparse
import protos.localization_protos_pb2 as loc_protos


def loadFeature(filename):
    f = open(filename, "rb")
    feature_proto = loc_protos.Feature()
    feature_proto.ParseFromString(f.read())
    f.close()

    return np.array(feature_proto.values)


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


def write_cost_matrix(cost_matrix, cost_matrix_file):

    cost_matrix_proto = loc_protos.CostMatrix()
    cost_matrix_proto.rows = cost_matrix.shape[0]
    cost_matrix_proto.cols = cost_matrix.shape[1]
    for row in cost_matrix:
        cost_matrix_proto.values.extend(row.tolist())

    f = open(cost_matrix_file, "wb")
    f.write(cost_matrix_proto.SerializeToString())
    f.close()


def load_cost_matrix(cost_matrix_file):
    f = open(cost_matrix_file, "rb")
    cost_matrix_proto = loc_protos.CostMatrix()
    cost_matrix_proto.ParseFromString(f.read())
    f.close()
    cost_matrix = np.array(cost_matrix_proto.values)
    cost_matrix = np.reshape(
        cost_matrix, (cost_matrix_proto.rows, cost_matrix_proto.cols)
    )
    print("Retrieved cost_matrix", cost_matrix)
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
    for qu in query_files:
        query_features.append(loadFeature(qu))

    db_features = []
    for db in db_files:
        db_features.append(loadFeature(db))

    cost_matrix = compute_cost_matrix(query_features, db_features)

    print("Matrix size", cost_matrix.shape)
    write_cost_matrix(cost_matrix, cost_matrix_file)
    print("The cost matrix was save to", cost_matrix_file)


if __name__ == "__main__":
    main()
