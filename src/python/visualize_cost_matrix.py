import numpy as np
import cv2
import argparse
from pathlib import Path
import protos_io


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--cost_matrix",
        required=True,
        type=Path,
        help="Path to the cost_matrix .CostMatrix.pb file",
    )
    parser.add_argument(
        "--image_name",
        required=False,
        default=None,
        type=Path,
        help="Image name to save the cost matrix to.",
    )
    args = parser.parse_args()

    cost_matrix_file = args.cost_matrix
    cost_matrix = protos_io.read_cost_matrix(cost_matrix_file)

    max_value = np.max(np.max(cost_matrix))
    min_value = np.min(np.min(cost_matrix))

    print("Max value", max_value)
    print("Min value", min_value)

    if args.image_name:
        print(args.image_name)
        img = np.array(cost_matrix, dtype=float) * float(255)
        cv2.imwrite(str(args.image_name), img)

    window_name = f"cost_matrix {max_value:.4f}:{min_value:.4f}"

    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.imshow(window_name, cost_matrix)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
