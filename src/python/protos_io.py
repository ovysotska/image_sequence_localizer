import numpy as np
import protos.localization_protos_pb2 as loc_protos


def read_feature(feature_filename):
    f = open(feature_filename, "rb")
    feature_proto = loc_protos.Feature()
    feature_proto.ParseFromString(f.read())
    f.close()

    return np.array(feature_proto.values)


def write_feature(filename, proto):
    f = open(filename, "wb")
    f.write(proto.SerializeToString())
    f.close()


def write_cost_matrix(cost_matrix, cost_matrix_file):
    cost_matrix_proto = loc_protos.CostMatrix()
    cost_matrix_proto.rows = cost_matrix.shape[0]
    cost_matrix_proto.cols = cost_matrix.shape[1]
    for row in cost_matrix:
        cost_matrix_proto.values.extend(row.tolist())

    f = open(cost_matrix_file, "wb")
    f.write(cost_matrix_proto.SerializeToString())
    f.close()


def read_cost_matrix(cost_matrix_file):
    f = open(cost_matrix_file, "rb")
    cost_matrix_proto = loc_protos.CostMatrix()
    cost_matrix_proto.ParseFromString(f.read())
    f.close()
    cost_matrix = np.array(cost_matrix_proto.values)
    cost_matrix = np.reshape(
        cost_matrix, (cost_matrix_proto.rows, cost_matrix_proto.cols)
    )
    return cost_matrix


def read_matching_result(filename):
    f = open(filename, "rb")
    result_proto = loc_protos.MatchingResult()
    result_proto.ParseFromString(f.read())
    f.close()
    return result_proto


def read_expanded_mask(expanded_patches_dir):
    matching_costs_files = list(expanded_patches_dir.glob("*.MatchingCosts.pb"))
    matching_costs_files.sort()

    mask = []
    for matching_costs_file in matching_costs_files:
        f = open(matching_costs_file, "rb")
        matching_costs_proto = loc_protos.MatchingCosts()
        matching_costs_proto.ParseFromString(f.read())
        f.close()
        mask.extend(matching_costs_proto.elements)
    return mask
