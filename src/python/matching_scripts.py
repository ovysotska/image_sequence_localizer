from pathlib import Path
import os
from dataclasses import dataclass, fields, asdict, replace


@dataclass
class RunParameters:
    path2query_images: str = None
    path2ref_images: str = None
    path2qu: str = None
    path2ref: str = None
    similarityMatrix: str = None
    matchingResult: str = None
    matchingResultImage: str = None
    expansionRate: float = 0.3
    fanOut: int = 5
    matchingThreshold: float = 3.7
    querySize: int = None
    bufferSize: int = 100


def initializeFromDict(params, params_as_dict):
    init_obj = replace(params, **params_as_dict)
    return init_obj


def convertToDictWithoutNoneEntries(params):
    # Iterate over the fields and check if any are None
    for field in fields(params):
        field_value = getattr(params, field.name)  # Get the value of the field
        if field_value is None:
            print(f"Field '{field.name}' is None")

    # Remove None entries
    params_as_dict = asdict(params)
    dict_without_none = {
        key: value for key, value in params_as_dict.items() if value is not None
    }
    return dict_without_none


def convertNumpyFeaturesToProtoAndWrite(
    np_features, output_folder, dataset_name, feature_type
):
    params = "--filename {features_file} ".format(features_file=np_features)
    params += "--feature_type {feature_type} ".format(feature_type=feature_type)
    params += "--output_folder {output_folder} ".format(output_folder=output_folder)
    params += "--output_file_prefix {output_file_prefix} ".format(
        output_file_prefix=dataset_name
    )

    command = "python convert_numpy_features_to_protos.py " + params
    print("Calling:", command)
    os.system(command)


def computeNetVLADFeatures(image_dir, output_folder):
    netvlad_weights = Path("netvlad/data/Pitts30K_struct.mat")
    if not netvlad_weights.exists():
        print(
            "ERROR: Can't find netvlad weights. Please download to netvlad/data/ from https://cvg-data.inf.ethz.ch/hloc/netvlad/Pitts30K_struct.mat"
        )
        exit(1)

    feature_name_prefix = "NetVLAD"
    params = "--data_dir {image_dir} ".format(image_dir=image_dir)
    np_features_file = output_folder / (feature_name_prefix + "-features.txt")
    params += "--output_file {output_file} ".format(output_file=np_features_file)
    params += "--netvlad_weights_file {weights} ".format(weights=netvlad_weights)

    command = "python netvlad/netvlad_extractor.py  " + params
    print("Calling:", command)
    os.system(command)
    return np_features_file


def computeFeatures(image_dir, output_folder, dataset_name, feature_type):
    if output_folder.exists():
        print(
            "WARNING: Feature folder {folder_name} exists. Skipping feature computation.".format(
                folder_name=output_folder
            )
        )
        return
    output_folder.mkdir()

    if feature_type == "NetVLAD":
        print("Computing NetVLAD features")
        np_features_file = computeNetVLADFeatures(image_dir, output_folder)
    else:
        print("ERROR. Unrecognized feature type.")
        exit(1)

    convertNumpyFeaturesToProtoAndWrite(
        np_features_file, output_folder, dataset_name, feature_type
    )


def computeSimilarityMatrix(run_params):
    params = "--query_features {query_features} ".format(
        query_features=run_params.path2qu
    )
    params += "--db_features {reference_features} ".format(
        reference_features=run_params.path2ref
    )
    params += "--similarity_matrix_file {cost_matrix_file} ".format(
        cost_matrix_file=run_params.similarityMatrix
    )
    command = "python compute_similarity_matrix.py " + params
    print("Calling:", command)
    os.system(command)


def runMatching(config_yaml_file):
    binary = "../../build/src/apps/cost_matrix_based_matching/online_localizer_lsh"
    command = binary + " " + str(config_yaml_file)
    print("Calling:", command)
    os.system(command)


def runLocalizationResultVisualization(run_params):
    params = "--similarity_matrix {similarity_matrix} ".format(cost_matrix=run_params.similarityMatrix)
    params += "--matching_result {matching_result} ".format(
        matching_result=run_params.matchingResult
    )
    params += "--image_name {image_name} ".format(
        image_name=run_params.matchingResultImage
    )
    command = "python visualize_localization_result.py " + params
    print("Calling:", command)
    os.system(command)


def runStoreImageMatches(run_params, output_dir):
    params = "--matching_result {matching_result} ".format(
        matching_result=run_params.matchingResult
    )
    params += "--query_images {query_images} ".format(
        query_images=run_params.path2query_images
    )
    params += "--reference_images {ref_images} ".format(
        ref_images=run_params.path2ref_images
    )
    params += "--output_dir {output_dir}/matched_images ".format(output_dir=output_dir)

    command = "python store_image_matches.py " + params
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
