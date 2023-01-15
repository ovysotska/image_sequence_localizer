import argparse
from pathlib import Path
import os
import yaml


def parseParams():
    parser = argparse.ArgumentParser(description="Run image matching.")
    parser.add_argument(
        "--query_features",
        type=Path,
        required=True,
        help="Path to the directory with features of type .Feature.pb",
    )
    parser.add_argument(
        "--reference_features",
        type=Path,
        required=True,
        help="Path to the directory with features of type .Feature.pb",
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


def setDictParam(args):
    params = dict()
    params["path2qu"] = args.query_features.as_posix()
    params["path2ref"] = args.reference_features.as_posix()
    params["costMatrix"] = (
        args.output_dir / (args.dataset_name + ".CostMatrix.pb")
    ).as_posix()
    params["matchingResult"] = (
        args.output_dir / (args.dataset_name + ".MatchingResult.pb")
    ).as_posix()
    params["matchingResultImage"] = (
        args.output_dir / (args.dataset_name + "_result.png")
    ).as_posix()
    params["expansionRate"] = 0.3
    params["fanOut"] = 5
    params["nonMatchCost"] = 3.7
    params["bufferSize"] = 100

    queriesNum = len(list(args.query_features.glob("*Feature.pb")))
    params["querySize"] = queriesNum
    return params


def computeCostMatrix(config):
    params = "--query_features {query_features} ".format(
        query_features=config["path2qu"]
    )
    params += "--db_features {reference_features} ".format(
        reference_features=config["path2ref"]
    )
    params += "--cost_matrix_file {cost_matrix_file} ".format(
        cost_matrix_file=config["costMatrix"]
    )
    command = "python compute_cost_matrix.py " + params
    print("Calling:", command)
    os.system(command)


def runMatching(config_yaml_file):
    binary = (
        "../../build/src/apps/cost_matrix_based_matching/cost_matrix_based_matching_lsh"
    )
    command = binary + " " + config_yaml_file.as_posix()
    print("Calling:", command)
    os.system(command)


def runResultVisualization(config):
    params = "--cost_matrix {cost_matrix} ".format(cost_matrix=config["costMatrix"])
    params += "--matching_result {matching_result} ".format(
        matching_result=config["matchingResult"]
    )
    params += "--image_name {image_name} ".format(
        image_name=config["matchingResultImage"]
    )
    command = "python visualize_localization_result.py " + params
    print("Calling:", command)
    os.system(command)


def main():
    args = parseParams()
    yaml_config = setDictParam(args)

    if args.output_dir.exists():
        print("WARNING: output_dir exists. Overwritting the results")
    else:
        args.output_dir.mkdir()

    yaml_config_file = args.output_dir / (args.dataset_name + "_config.yml")
    with open(yaml_config_file, "w") as file:
        yaml.dump(yaml_config, file)
    computeCostMatrix(yaml_config)
    runMatching(yaml_config_file)
    runResultVisualization(yaml_config)
    # TODO(olga): Add visualize_matching_results.py


if __name__ == "__main__":
    main()
