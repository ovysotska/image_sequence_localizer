import matching_scripts as matching

import yaml

import argparse
from pathlib import Path


def parseParams():
    parser = argparse.ArgumentParser(
        description="Run image matching starting from features."
    )
    parser.add_argument(
        "--query_features",
        type=Path,
        required=True,
        help="Path to the directory with features, e.g., .NetVLAD.Feature.pb",
    )
    parser.add_argument(
        "--reference_features",
        type=Path,
        required=True,
        help="Path to the directory with features, e.g., .NetVLAD.Feature.pb",
    )
    parser.add_argument(
        "--dataset_name",
        type=str,
        required=True,
        help="The name of the dataset.",
    )
    parser.add_argument(
        "--output_dir",
        type=Path,
        required=True,
        help="Path to output directory to store results.",
    )
    return parser.parse_args()


def setRunParameters(args):
    run_parameters = matching.RunParameters()
    run_parameters.path2qu = args.query_features.as_posix()
    run_parameters.path2ref = args.reference_features.as_posix()
    run_parameters.similarityMatrix = (
        args.output_dir / (args.dataset_name + ".SimilarityMatrix.pb")
    ).as_posix()
    run_parameters.matchingResult = (
        args.output_dir / (args.dataset_name + ".MatchingResult.pb")
    ).as_posix()
    run_parameters.matchingResultImage = (
        args.output_dir / (args.dataset_name + "_result.png")
    ).as_posix()
    run_parameters.debugProto = (
        args.output_dir / (args.dataset_name + ".OnlineLocalizerDebug.pb")
    ).as_posix()

    queriesNum = len(list(args.query_features.glob("*Feature.pb")))
    run_parameters.querySize = queriesNum

    return run_parameters


def main():
    args = parseParams()

    if args.output_dir.exists():
        print("WARNING: output_dir exists. Overwritting the results")
    else:
        args.output_dir.mkdir()

    run_params = setRunParameters(args)
    param_as_dict = matching.convertToDictWithoutNoneEntries(run_params)

    yaml_config_file = args.output_dir / (args.dataset_name + "_config.yml")
    with open(yaml_config_file, "w") as file:
        yaml.dump(param_as_dict, file)

    matching.computeSimilarityMatrix(run_params)
    matching.runMatching(yaml_config_file)
    matching.runLocalizationResultVisualization(run_params)


if __name__ == "__main__":
    main()
