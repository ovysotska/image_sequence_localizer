import pytest
import numpy as np
from pathlib import Path

import convert_numpy_features_to_protos as converter


def test_convert_to_protos():
    features = np.array([[1, 2, 3], [4, 5, 6]])
    feature_type = "Test"

    protos = converter.convert_to_protos(features, feature_type)
    np.testing.assert_equal(protos[0].values, features[0, :])
    np.testing.assert_equal(protos[1].values, features[1, :])

    assert protos[0].type == feature_type
    assert protos[1].type == feature_type

    assert protos[0].size == 3
    assert protos[1].size == 3


def test_convert_to_proto_not_valid():
    features = np.zeros((2, 3, 4))
    with pytest.raises(AssertionError):
        protos = converter.convert_to_protos(features, "Test")


def test_save_protos_to_files(tmp_path):
    features = np.array([[1, 2, 3], [4, 5, 6]])
    feature_type = "Test"
    protos = converter.convert_to_protos(features, feature_type)
    output_folder = tmp_path / Path("result")
    converter.save_protos_to_files(output_folder, protos, "feature")

    assert (output_folder / "feature_0000000.Test.Feature.pb").exists()
    assert (output_folder / "feature_0000001.Test.Feature.pb").exists()
    assert len(list(output_folder.glob("*.pb"))) == 2
