import numpy as np
import cv2
import argparse
from pathlib import Path
import matplotlib.pyplot as plt

from sklearn import preprocessing
from sklearn.cluster import KMeans
from sklearn.neighbors import KDTree

kDefaultWidth = 640  # px
kDefaultClusterSize = 400


def listImagesInFolder(folderPath):
    trainImageFiles = list(folderPath.glob("*.jpg"))
    trainImageFiles.extend(list(folderPath.glob("*.png")))
    return trainImageFiles


def rescaleImageIfNeeded(image):
    height, width = image.shape
    if width > kDefaultWidth:
        newHeight = (height * kDefaultWidth) / width
        image = cv2.resize(image, (kDefaultWidth, int(newHeight)))
        print("Resized image from", height, width, "to", image.shape)
    return image


def extractSiftsFromImage(imageFile):
    """Extracts SIFT features from an image

    Args:
        imageFile (Path): path to the image file

    Returns:
        list(list(int)): array of descriptors NxD
    """
    # Extracts features from an image
    image = cv2.imread(imageFile.as_posix(), cv2.IMREAD_GRAYSCALE)
    image = rescaleImageIfNeeded(image)
    sift = cv2.SIFT_create()
    keypoints, descriptors = sift.detectAndCompute(image, None)
    return descriptors


def computeIDF(descriptorsPerImage, clusters):
    """Compute inverse document frequence (IDF). Here means in how many images does the word occur.

    Args:
        descriptorsByImages (list(list(1xD)): List of descriptors per image
        clusters (np.array): CxD array of clusters (words)
    Returns:
        np.array: Cx1 occurence of clusters/words in images
    """
    clusterOccurenceInImages = [set() for index in range(clusters.shape[0])]
    N = len(descriptorsPerImage)
    clustersTree = KDTree(clusters)
    for imageId in range(len(descriptorsPerImage)):
        dist, nearestClusters = clustersTree.query(descriptorsPerImage[imageId], k=1)
        for clusterId in nearestClusters.squeeze():
            if clusterId < 0 or clusterId >= clusters.shape[0]:
                print("Error: cluster ids outside bounds")
                continue
            clusterOccurenceInImages[clusterId].add(imageId)

    # reweight by number of images
    clusterOccurence = [0] * clusters.shape[0]
    for clusterId in range(len(clusterOccurenceInImages)):
        if len(clusterOccurenceInImages[clusterId]) <= 0:
            print("WARNING: word", clusterId, "is not represented in any image")
            continue
        clusterOccurence[clusterId] = N / len(clusterOccurenceInImages[clusterId])
    return np.array(clusterOccurence)


def trainVocabulary(imageFiles, outputFile=""):
    """train vocabulary from given image paths

    Args:
        imageFiles (list(Path)): paths to images
    """
    descriptorsPerImage = []
    for imageFile in imageFiles:
        sifts = extractSiftsFromImage(imageFile)
        descriptorsPerImage.append(sifts)

    # flatten the descriptors list
    descriptors = [
        descriptor for descriptors in descriptorsPerImage for descriptor in descriptors
    ]
    descriptors = np.array(descriptors)

    descriptorsNormalized = preprocessing.normalize(descriptors)
    kmeans = KMeans(n_clusters=kDefaultClusterSize, random_state=0, n_init="auto")
    kmeans.fit(descriptorsNormalized)
    words = kmeans.cluster_centers_

    idfs = computeIDF(descriptorsPerImage, words)

    plt.bar(range(0, len(idfs)), idfs)
    plt.savefig("idf_" + str(kDefaultClusterSize) + ".png")

    if outputFile:
        np.savez(outputFile, vocabulary=words, idfs=idfs)
        print("Vocabulary was saved to", outputFile)
    return words, idfs


def trainVocabularyFromFolder(folderPath, outputFile=""):
    return trainVocabulary(listImagesInFolder(folderPath), outputFile)


def getVocabulary(imageTrainFolder, vocabularyFile):
    if vocabularyFile is not None:
        if vocabularyFile.exists():
            print("Vocabulary exists and will be loaded")
            data = np.load(vocabularyFile)
            return data["vocabulary"], data["idfs"]
        elif imageTrainFolder is None:
            print("Vocabulary doesn't exits, please provide images to train on")
            return None
        else:
            return trainVocabularyFromFolder(imageTrainFolder, vocabularyFile)
    elif imageTrainFolder:
        return trainVocabularyFromFolder(imageTrainFolder)
    else:
        print("No vocabulary or image_train data is provided.")
        return None


def reweightHistogram(wordOccurences, idfs):
    """Reweight Histogram

    Args:
        wordOccurences (np.array): Cx1 array
        idfs (np.array): Cx1 array, inverse document frequency (idf). How often every word occurres in training database.

    Returns:
        np.array: Reweigted histogram
    """
    totalNumberOfWordOccurences = np.sum(wordOccurences)
    reweightedHistogram = np.zeros(wordOccurences.shape)
    for idx in range(wordOccurences.shape[0]):
        if idx < 0 or idx >= idfs.shape[0]:
            print("Error: index is outside the idfs range")
            continue
        reweightedHistogram[idx] = (
            wordOccurences[idx] / totalNumberOfWordOccurences * np.log(idfs[idx])
        )
    return reweightedHistogram


def computeImageHistogram(imagePath, vocabularyTree, numberOfWords, idfs):
    """Compute histogram of visual word occurence.

    Args:
        image (Path): Path to an image
        vocabularyTree (np.array): Array of words, CxD where C is the number of clusters
        numberOfWords (int) : Number of words in vocabulary
        idfs (np.array): Cx1 array of "learned" word occurence
    """
    wordHistogram = [0] * numberOfWords
    descriptors = extractSiftsFromImage(imagePath)
    if descriptors is None:
        print("Descriptors are empty", descriptors)
        return wordHistogram
    descriptorsNormalized = preprocessing.normalize(descriptors)

    for descriptor in descriptorsNormalized:
        dist, wordId = vocabularyTree.query(descriptor.reshape(1, -1), k=1)
        wordHistogram[np.squeeze(wordId)] += 1
    return reweightHistogram(np.array(wordHistogram), idfs)


def main():
    parser = argparse.ArgumentParser("Compute Bag Of visual Words (BoW) with SIFT.")
    parser.add_argument("--image_train_dir", required=False, type=Path)
    parser.add_argument("--vocabulary_file", required=False, type=Path)
    parser.add_argument(
        "--images",
        required=False,
        type=Path,
        help="Path to the image directory for which the histograms should be computed.",
    )
    parser.add_argument(
        "--outputFile",
        required=False,
        type=Path,
        help="Filename where Bow features will be stored, .csv recommended.",
    )

    args = parser.parse_args()

    vocabulary, idfs = getVocabulary(args.image_train_dir, args.vocabulary_file)

    numberOfWords = vocabulary.shape[0]
    vocabularyTree = KDTree(vocabulary)
    if args.images:
        if not args.outputFile:
            print(
                "WARNING: The output file is not specified. The features will not be stored."
            )
        imagesPath = listImagesInFolder(args.images)
        imagesPath = sorted(imagesPath)
        # TODO(olga) Make sure that the order is preserved by using a map of something. Would be better even to use the image name
        histograms = []
        for imagePath in imagesPath:
            print("Processing", imagePath)
            histogram = computeImageHistogram(
                imagePath, vocabularyTree, numberOfWords, idfs
            )
            histograms.append(histogram)
            print("Processing done")
        histograms = np.array(histograms)
        if args.outputFile:
            np.savetxt(args.outputFile, histograms)
            print("Features were saved to", args.outputFile)

    return


if __name__ == "__main__":
    main()
