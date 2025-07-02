import numpy as np
import cv2
import argparse
from pathlib import Path
import protos_io


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--similarity_matrix",
        required=True,
        type=Path,
        help="Path to the similarity matrix .SimilarityMatrix.pb file",
    )
    parser.add_argument(
        "--image_name",
        required=False,
        default=None,
        type=Path,
        help="Image name to save the similarity matrix to.",
    )
    args = parser.parse_args()

    similarity_matrix = protos_io.read_similarity_matrix(args.similarity_matrix)

    max_value = np.max(np.max(similarity_matrix))
    min_value = np.min(np.min(similarity_matrix))

    print("Max value", max_value)
    print("Min value", min_value)

    if args.image_name:
        print(args.image_name)
        img = np.array(similarity_matrix, dtype=float) * float(255)
        cv2.imwrite(str(args.image_name), img)

    window_name = f"similarity_matrix {max_value:.4f}:{min_value:.4f}"

    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.imshow(window_name, similarity_matrix)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
