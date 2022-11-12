import numpy as np
from pathlib import Path
import argparse
import protos_io

import protos.localization_protos_pb2 as loc_protos


def convert_to_protos(features, feature_type):
    """Converts the numpy nd array to the features protos.

    Args:
        features (numpy.ndarray): feature vectors to be converted.
                                Expected size NxD, where N is
                                number of features and D is number
                                of dimensions.
    Returns:
        [image_sequence_localizer.Feature]: list of feature protos.
                                            For details check
                                            localization_protos.proto file.
    """
    protos = []
    assert len(features.shape) == 2
    for feature in features:
        feature_proto = loc_protos.Feature()
        feature_proto.size = feature.shape[0]
        feature_proto.type = feature_type
        feature_proto.values.extend(feature)
        protos.append(feature_proto)
    return protos


def save_protos_to_files(folder, protos, prefix):
    if folder.exists():
        print("WARNING: the folder exists. Potentially overwritting the files.")
    else:
        folder.mkdir()
    for idx, proto in enumerate(protos):
        feature_idx = "{0:07d}".format(idx)
        proto_file = prefix + "_" + feature_idx + "." + proto.type + ".Feature.pb"
        protos_io.write_feature(folder / proto_file, proto)
        print("Feature", idx, "was written to ", folder / proto_file)
    return


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--filename",
        required=True,
        type=Path,
        help="Path to file that contains features in .txt format. "
        "Expected format is NxD, where N is number of features and D is number of dimensions.",
    )
    parser.add_argument(
        "--feature_type",
        required=True,
        type=str,
        help="Type of the features, e.g. NetVLAD.",
    )
    parser.add_argument(
        "--output_folder", required=True, type=Path, help="Path to db directory."
    )
    parser.add_argument(
        "--output_file_prefix",
        required=False,
        type=str,
        default="feature",
        help="Prefix for every feature file that will be generated.",
    )

    args = parser.parse_args()
    print("== Converting numpy matrix of features to protos ==")
    try:
        features = np.loadtxt(args.filename)
    except:
        print("ERROR: Could not read features from", args.filename)
        return
    protos = convert_to_protos(features, args.feature_type)
    save_protos_to_files(args.output_folder, protos, args.output_file_prefix)


if __name__ == "__main__":
    main()
