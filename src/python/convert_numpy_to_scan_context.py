import numpy as np
from pathlib import Path
import argparse
import protos_io

import protos.localization_protos_pb2 as loc_protos


def convert_to_protos(features):
    """Converts the numpy nd array to the features protos.

    Args:
        features (numpy.ndarray): feature vectors to be converted.
                                Expected size NxSxRxC, where N is
                                number of features and S is number
                                of shifts, R number of rows and C number of columns.
    Returns:
        [image_sequence_localizer.ScanContext]: list of feature protos.
                                            For details check
                                            localization_protos.proto file.
    """
    protos = []
    print("Number of features", features.shape)
    assert (
        len(features.shape) == 4
    ), "Expected a N x shifts x rows x cols dimensional matrix."
    for feature in features:
        scanContext_proto = loc_protos.ScanContext()
        for shift in feature:
            grid_proto = loc_protos.ScanContext.Grid()
            grid_proto.rows = shift.shape[0]
            grid_proto.cols = shift.shape[1]
            for r in range(grid_proto.rows):
                for c in range(grid_proto.cols):
                    grid_proto.values.extend([shift[r][c]])
            scanContext_proto.grids.extend([grid_proto])
        protos.append(scanContext_proto)
    return protos


def save_protos_to_files(folder, protos, type, prefix):
    if folder.exists():
        print("WARNING: the folder exists. Potentially overwritting the files.")
    else:
        folder.mkdir()
    for idx, proto in enumerate(protos):
        feature_idx = "{0:07d}".format(idx)
        proto_file = "{prefix}_{feature_idx}.{type}.Feature.pb".format(
            prefix=prefix, feature_idx=feature_idx, type=type
        )
        protos_io.write_feature(folder / proto_file, proto)
        print("Feature", idx, "was written to ", folder / proto_file)
    return


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--filename",
        required=True,
        type=Path,
        help="Path to file that contains features that can be loaded with np.load()",
    )
    parser.add_argument(
        "--feature_type",
        required=True,
        type=str,
        help="Type of the features, e.g. ScanContext",
    )
    parser.add_argument(
        "--output_folder", required=True, type=Path, help="Path to output directory"
    )
    parser.add_argument(
        "--output_file_prefix",
        required=False,
        type=str,
        default="scanContext",
        help="Prefix for every feature file that will be generated",
    )

    args = parser.parse_args()
    try:
        features = np.load(args.filename)
    except:
        print("ERROR: Could not read features from", args.filename)
        return
    print("There are: ", len(features))
    protos = convert_to_protos(features)
    save_protos_to_files(
        args.output_folder, protos, args.feature_type, args.output_file_prefix
    )


if __name__ == "__main__":
    main()
