from .bow import computeIDF
from .bow import reweightHistogram

import numpy as np
import pytest


def test_computeIDF():
    # 3 images with 2 descriptors of dimension 2
    descriptorsPerImage = [[[1, 2], [5, 6]], [[0, 0], [6, 5]], [[0, 0], [10, 9]]]
    # 3 clusters
    clusters = np.array([[0, 0], [6, 7], [10, 10]])
    occurance = computeIDF(descriptorsPerImage, clusters)
    np.testing.assert_array_almost_equal(occurance, [1.0, 1.5, 3.0])


def test_reweightHistogram():
    wordOccurences = np.array([5, 2, 1, 0, 0])
    idfs = 4 / np.array([4, 3, 4, 1, 1])
    reweightedHistogram = reweightHistogram(wordOccurences, idfs)
    np.testing.assert_array_almost_equal(reweightedHistogram, [0, 0.07192052, 0, 0, 0])
