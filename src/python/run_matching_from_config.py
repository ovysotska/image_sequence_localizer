import matching_scripts as matching

import yaml

import argparse


def parseParams():
    parser = argparse.ArgumentParser(
        description="Run image matching starting from config yaml file."
    )

    parser.add_argument(
        "--yaml_config_file",
        type=str,
        required=True,
        help="The yaml config file.",
    )
    return parser.parse_args()


def main():
    args = parseParams()

    with open(args.yaml_config_file, "r") as file:
        config = yaml.load(file, Loader=yaml.SafeLoader)

    run_params = matching.RunParameters()
    run_params = matching.initializeFromDict(run_params, config)

    matching.computeSimilarityMatrix(run_params)
    matching.runMatching(args.yaml_config_file)
    matching.runLocalizationResultVisualization(run_params)


if __name__ == "__main__":
    main()
