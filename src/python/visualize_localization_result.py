import numpy as np
import cv2
import argparse
from pathlib import Path
from compute_cost_matrix import load_cost_matrix
import protos.localization_protos_pb2 as loc_protos
import matplotlib.pyplot as plt


def load_matching_result(filename):
    f = open(filename, "rb")
    result_proto = loc_protos.MatchingResult()
    result_proto.ParseFromString(f.read())
    f.close()
    return result_proto


def show_matching_result(matching_result, cost_matrix):

    rgb_costs = np.zeros((cost_matrix.shape[0], cost_matrix.shape[1], 3))
    rgb_costs[:, :, 0] = cost_matrix
    rgb_costs[:, :, 1] = cost_matrix
    rgb_costs[:, :, 2] = cost_matrix

    for match in matching_result.matches:
        if match.real == True:
            rgb_costs[match.query_id, match.ref_id] = [1, 0, 0]
        else:
            rgb_costs[match.query_id, match.ref_id] = [0, 0, 1]
    plt.imshow(rgb_costs, vmin=0, vmax=1)
    plt.show()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--cost_matrix",
        required=True,
        type=Path,
        help="Path to the cost_matrix .CostMatrix.pb file",
    )
    parser.add_argument(
        "--matching_result",
        required=True,
        type=Path,
        help="Path to the matching result .MatchingResult.pb file",
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
    cost_matrix = load_cost_matrix(cost_matrix_file)

    max_value = np.max(np.max(cost_matrix))
    min_value = np.min(np.min(cost_matrix))

    print("Max value", max_value)
    print("Min value", min_value)

    matching_result = load_matching_result(args.matching_result)

    show_matching_result(matching_result, cost_matrix)

    # if args.image_name:
    #     print(args.image_name)
    #     img = np.array(cost_matrix, dtype=float) * float(255)
    #     cv2.imwrite(str(args.image_name), img)

    # window_name = f"cost_matrix {max_value:.4f}:{min_value:.4f}"

    # cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    # cv2.imshow(window_name, cost_matrix)
    # cv2.waitKey(0)
    # cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
