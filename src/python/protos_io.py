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


def write_similarity_matrix(simlarity_matrix, similarity_matrix_file):

    similarity_matrix_proto = loc_protos.SimilarityMatrix()
    similarity_matrix_proto.rows = simlarity_matrix.shape[0]
    similarity_matrix_proto.cols = simlarity_matrix.shape[1]
    for row in simlarity_matrix:
        similarity_matrix_proto.values.extend(row.tolist())

    f = open(similarity_matrix_file, "wb")
    f.write(similarity_matrix_proto.SerializeToString())
    f.close()


def read_similarity_matrix(similarity_matrix_file):
    f = open(similarity_matrix_file, "rb")
    similarity_matrix_proto = loc_protos.CostMatrix()
    similarity_matrix_proto.ParseFromString(f.read())
    f.close()
    similarity_matrix = np.array(similarity_matrix_proto.values)
    similarity_matrix = np.reshape(
        similarity_matrix, (similarity_matrix_proto.rows, similarity_matrix_proto.cols)
    )
    return similarity_matrix


def read_matching_result(filename):
    f = open(filename, "rb")
    result_proto = loc_protos.MatchingResult()
    result_proto.ParseFromString(f.read())
    f.close()
    return result_proto


def read_expanded_mask(expanded_patches_dir):
    patch_files = list(expanded_patches_dir.glob("*.Patch.pb"))
    patch_files.sort()

    mask = []
    for patch_file in patch_files:
        f = open(patch_file, "rb")
        patch_proto = loc_protos.Patch()
        patch_proto.ParseFromString(f.read())
        f.close()
        mask.extend(patch_proto.elements)
    return mask
