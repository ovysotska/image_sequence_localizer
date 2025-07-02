import numpy as np
from pathlib import Path
import argparse
import protos_io


def cosine_similarity(qu, db):
    norm = np.linalg.norm(qu) * np.linalg.norm(db)
    dist = np.dot(qu, db) / norm
    return dist


def compute_similarity_matrix(qu_features, db_features):
    similarity_matrix = np.zeros((len(qu_features), len(db_features)))

    print("Computing similarity matrix...")
    for q, query_feature in enumerate(qu_features):
        for d, db_feature in enumerate(db_features):
            # Compute cosine distance
            similarity_matrix[q, d] = cosine_similarity(query_feature, db_feature)

    print("Finished.")
    return similarity_matrix


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--query_features", required=True, type=Path, help="Path to query directory."
    )
    parser.add_argument(
        "--db_features", required=True, type=Path, help="Path to db directory."
    )
    parser.add_argument(
        "--similarity_matrix_file",
        required=False,
        default="similarity_matrix.SimilarityMatrix.pb",
        type=Path,
        help="File name for similarity matrix output with extension SimilarityMatrix.pb",
    )
    args = parser.parse_args()

    query_files = list(args.query_features.glob("*.Feature.pb"))
    db_files = list(args.db_features.glob("*.Feature.pb"))
    similarity_matrix_file = args.similarity_matrix_file

    if len(query_files) == 0:
        print("WARNING: no query features were read")
        return
    if len(db_files) == 0:
        print("WARNING: no reference features were read")
        return

    query_files.sort()
    db_files.sort()

    query_features = []
    for query_file in query_files:
        query_features.append(protos_io.read_feature(query_file))

    db_features = []
    for db_file in db_files:
        db_features.append(protos_io.read_feature(db_file))

    similarity_matrix = compute_similarity_matrix(query_features, db_features)

    print("Matrix size", similarity_matrix.shape)
    protos_io.write_similarity_matrix(similarity_matrix, similarity_matrix_file)
    print("The similarity matrix was save to", similarity_matrix_file)


if __name__ == "__main__":
    main()
