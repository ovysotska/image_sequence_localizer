import matching_scripts as matching

import yaml

import argparse
from pathlib import Path


def parseParams():
    parser = argparse.ArgumentParser(
        description="Run image matching starting from images."
    )
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
        "--feature_type",
        type=str,
        choices=("NetVLAD", "Other"),
        default="NetVLAD",
        help="Feature type to be extracted from images.",
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


def setRunParameters(args):
    run_parameters = matching.RunParameters()
    run_parameters.path2query_images = args.query_images.as_posix()
    run_parameters.path2ref_images = args.reference_images.as_posix()
    run_parameters.path2qu = (args.output_dir / "query_features").as_posix()
    run_parameters.path2ref = (args.output_dir / "reference_features").as_posix()
    run_parameters.similarityMatrix = (
        args.output_dir / (args.dataset_name + ".SimilarityMatrix.pb")
    ).as_posix()
    run_parameters.matchingResult = (
        args.output_dir / (args.dataset_name + ".MatchingResult.pb")
    ).as_posix()
    run_parameters.matchingResultImage = (
        args.output_dir / (args.dataset_name + "_result.png")
    ).as_posix()

    queriesNum = len(list(Path(run_parameters.path2qu).glob("*Feature.pb")))
    run_parameters.querySize = queriesNum

    return run_parameters


def main():
    args = parseParams()

    if args.output_dir.exists():
        print("WARNING: output_dir exists. Overwritting the results")
    else:
        args.output_dir.mkdir()

    if args.link_images:
        matching.linkImages(args.query_images, args.output_dir / "query_images")
        matching.linkImages(args.reference_images, args.output_dir / "reference_images")

    # Compute query features.
    query_features_dir = args.output_dir / "query_features"
    matching.computeFeatures(
        args.query_images, query_features_dir, args.dataset_name, args.feature_type
    )

    # Compute reference features.
    reference_features_dir = args.output_dir / "reference_features"
    matching.computeFeatures(
        args.reference_images,
        reference_features_dir,
        args.dataset_name,
        args.feature_type,
    )

    run_params = setRunParameters(args)
    params_as_dict = matching.convertToDictWithoutNoneEntries(run_params)

    yaml_config_file = args.output_dir / (args.dataset_name + "_config.yml")
    with open(yaml_config_file, "w") as file:
        yaml.dump(params_as_dict, file)

    matching.computeSimilarityMatrix(run_params)
    matching.runMatching(yaml_config_file)
    matching.runLocalizationResultVisualization(run_params)
    if args.write_image_matches:
        matching.runStoreImageMatches(run_params, args.output_dir)


if __name__ == "__main__":
    main()
