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
    parser.add_argument(
        "--write_image_matches",
        action="store_true",
        help="Creates and writes the pair of matching images.",
    )
    parser.add_argument(
        "--link_images",
        action="store_true",
        help="Creates hard link for images in the result folder.",
    )
    return parser.parse_args()


def setDictParam(args, query_features_dir, reference_features_dir):
    params = dict()
    params["path2query_images"] = str(args.query_images)
    params["path2ref_images"] = str(args.reference_images)
    params["path2qu"] = str(query_features_dir)
    params["path2ref"] = str(reference_features_dir)
    params["costMatrix"] = str(args.output_dir / (args.dataset_name + ".CostMatrix.pb"))
    params["matchingResult"] = str(
        args.output_dir / (args.dataset_name + ".MatchingResult.pb")
    )
    params["matchingResultImage"] = str(
        args.output_dir / (args.dataset_name + "_result.png")
    )
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
    binary = "../../build/src/apps/cost_matrix_based_matching/online_localizer_lsh"
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


def linkImages(image_dir, output_folder):
    if output_folder.exists():
        print(
            "Output {output_folder} exists. Skipping...".format(
                output_folder=output_folder
            )
        )
        return
    output_folder.mkdir()
    images = image_dir.glob("*")
    for image in images:
        image_link_name = output_folder / image.name
        command = "ln {image} {image_link_name}".format(
            image=image, image_link_name=image_link_name
        )
        print(command)
        os.system(command)
    print("Linked images from", image_dir)


def main():
    args = parseParams()

    if args.output_dir.exists():
        print("WARNING: output_dir exists. Overwritting the results")
    else:
        args.output_dir.mkdir()

    if args.link_images:
        linkImages(args.query_images, args.output_dir / "query_images")
        linkImages(args.reference_images, args.output_dir / "reference_images")

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
    if args.write_image_matches:
        runMatchingResultVisualization(yaml_config, args.output_dir)


if __name__ == "__main__":
    main()
