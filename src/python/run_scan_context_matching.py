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
        help="Path to the directory with images in .jpg or .png format",
    )
    parser.add_argument(
        "--reference_features",
        type=Path,
        required=True,
        help="Path to the directory with images in .jpg or .png format",
    )

    parser.add_argument(
        "--cost_matrix", type=Path, required=True, help="Path to precompute cost matrix"
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


def setDictParam(args, query_features_dir, reference_features_dir, cost_matrix_dir):
    params = dict()
    params["path2qu"] = str(query_features_dir)
    params["path2ref"] = str(reference_features_dir)
    params["costMatrix"] = str(cost_matrix_dir)
    params["matchingResult"] = str(
        args.output_dir / (args.dataset_name + ".MatchingResult.pb")
    )
    params["matchingResultImage"] = str(
        args.output_dir / (args.dataset_name + "_result.png")
    )
    params["expandedNodesFile"] = str(
        args.output_dir / (args.dataset_name + "_expanded_nodes.Patch.pb")
    )
    params["expansionRate"] = 0.7
    params["fanOut"] = 10
    params["nonMatchCost"] = 0.1
    params["bufferSize"] = 100

    queriesNum = len(list(query_features_dir.glob("*Feature.pb")))
    params["querySize"] = queriesNum
    return params


def runMatching(config_yaml_file):
    binary = "../../build/src/apps/cost_matrix_based_matching/scan_context_matching"
    command = binary + " " + str(config_yaml_file)
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

    # params += "--expanded_patches_file {file} ".format(file=config["expandedNodesFile"])
    command = "python visualize_localization_result.py " + params
    print("Calling:", command)
    os.system(command)


def runMatchingResultVisualization(config, output_dir):
    params = "--matching_result {matching_result} ".format(
        matching_result=config["matchingResult"]
    )
    params += "--query_images {query_images} ".format(
        query_images=config["path2query_images"]
    )
    params += "--reference_images {ref_images} ".format(
        ref_images=config["path2ref_images"]
    )
    params += "--output_dir {output_dir}/matched_images ".format(output_dir=output_dir)

    command = "python visualize_matching_result.py " + params
    print("Calling:", command)
    os.system(command)


def main():
    args = parseParams()

    if args.output_dir.exists():
        print("WARNING: output_dir exists. Overwritting the results")
    else:
        args.output_dir.mkdir()

    yaml_config = setDictParam(
        args, args.query_features, args.reference_features, args.cost_matrix
    )

    yaml_config_file = args.output_dir / (args.dataset_name + "_config.yml")
    with open(yaml_config_file, "w") as file:
        yaml.dump(yaml_config, file)
    runMatching(yaml_config_file)
    runResultVisualization(yaml_config)


if __name__ == "__main__":
    main()
