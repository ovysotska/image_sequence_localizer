import argparse
from pathlib import Path
import os
import yaml


def parseParams():
    parser = argparse.ArgumentParser(description="Run image matching.")
    parser.add_argument(
        "--query_images",
        type=Path,
        required=True,
        help="Path to the directory with images in .jpg or .png format",
    )
    parser.add_argument(
        "--reference_images",
        type=Path,
        required=True,
        help="Path to the directory with images in .jpg or .png format",
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


def setDictParam(args, query_features_dir, reference_features_dir):
    params = dict()
    params["path2qu"] = query_features_dir.as_posix()
    params["path2ref"] = reference_features_dir.as_posix()
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

    queriesNum = len(list(query_features_dir.glob("*Feature.pb")))
    params["querySize"] = queriesNum
    return params


def computeFeatures(image_dir, output_folder, feature_name_prefix):
    if output_folder.exists():
        print(
            "WARNING: Feature folder {folder_name} exists. Skipping feature computation.".format(
                folder_name=output_folder
            )
        )
        return
    output_folder.mkdir()
    # Extract features.
    netvlad_weights = Path("netvlad/data/Pitts30K_struct.mat")
    if not netvlad_weights.exists():
        print(
            "ERROR: Can't find netvlad weights. Please download to netvlad/data/ from https://cvg-data.inf.ethz.ch/hloc/netvlad/Pitts30K_struct.mat"
        )
        exit(1)

    params = "--data_dir {image_dir} ".format(image_dir=image_dir)
    np_features = output_folder / (feature_name_prefix + "-features.txt")
    params += "--output_file {output_file} ".format(output_file=np_features)
    params += "--netvlad_weights_file {weights} ".format(weights=netvlad_weights)

    command = "python netvlad/netvlad_extractor.py  " + params
    print("Calling:", command)
    os.system(command)

    # Convert from np to protos.
    params = "--filename {features_file} ".format(features_file=np_features)
    params += "--feature_type NetVLAD "
    params += "--output_folder {output_folder} ".format(output_folder=output_folder)
    params += "--output_file_prefix {output_file_prefix} ".format(
        output_file_prefix=feature_name_prefix
    )

    command = "python convert_numpy_features_to_protos.py " + params
    print("Calling:", command)
    os.system(command)


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

    if args.output_dir.exists():
        print("WARNING: output_dir exists. Overwritting the results")
    else:
        args.output_dir.mkdir()

    # Compute query features.
    query_features_dir = args.output_dir / "query_features"
    computeFeatures(args.query_images, query_features_dir, args.dataset_name)

    # Compute reference features.
    reference_features_dir = args.output_dir / "reference_features"
    computeFeatures(args.reference_images, reference_features_dir, args.dataset_name)

    yaml_config = setDictParam(args, query_features_dir, reference_features_dir)
    yaml_config_file = args.output_dir / (args.dataset_name + "_config.yml")
    with open(yaml_config_file, "w") as file:
        yaml.dump(yaml_config, file)
    computeCostMatrix(yaml_config)
    runMatching(yaml_config_file)
    runResultVisualization(yaml_config)
    # TODO(olga): Add visualize_matching_results.py


if __name__ == "__main__":
    main()
