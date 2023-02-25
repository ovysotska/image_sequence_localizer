# This file is highly inspired by
# https://github.com/cvg/Hierarchical-Localization/blob/master/hloc/extractors/netvlad.py
# The weights are downloaded from https://cvg-data.inf.ethz.ch/hloc/netvlad/Pitts30K_struct.mat
# The parsing of the .mat file is according to aforementioned project.

import torch
from torch import nn
from torchvision.transforms import ToTensor
import torchvision.models as models
from torch.utils.data import Dataset

from PIL import Image

from scipy.io import loadmat
import numpy as np

from pathlib import Path
import argparse

kNetVLADImageSize = 224


class ImageDataset(Dataset):
    def __init__(self, img_dir, transform=None):
        self.img_names = []
        self.img_names.extend(img_dir.glob("*.png"))
        self.img_names.extend(img_dir.glob("*.jpg"))
        self.img_names.sort()
        self.transform = transform

    def __len__(self):
        return len(self.img_names)

    def __getitem__(self, idx):
        assert idx >= 0 and idx < len(self.img_names)
        image = Image.open(str(self.img_names[idx]))
        image = image.resize((kNetVLADImageSize, kNetVLADImageSize))
        if self.transform:
            image = self.transform(image)
        # returns an image in the proper format
        return image


class VLADLayer(nn.Module):
    def __init__(
        self,
        inputDim=512,
        numberOfClusters=64,
    ):
        super().__init__()
        # Sort of clustering.
        # Assigning a feature and clusters with softmax assignment
        # Initialize the needed layers.
        self.clusterAssociations = torch.nn.Conv1d(
            inputDim, numberOfClusters, 1, stride=1
        )
        centers = torch.nn.parameter.Parameter(
            torch.empty([inputDim, numberOfClusters])
        )
        self.register_parameter("centers", centers)

    def forward(self, featureVolume):
        # featureVolume of size (1, D, N)
        batch = featureVolume.size(0)
        # Size (1, K, N)
        assignmentScores = self.clusterAssociations(featureVolume)
        softAssignments = nn.functional.softmax(assignmentScores, dim=1)

        # Reshape from (1, D, N) to (1, D, 1, N)
        featureTensor = featureVolume.unsqueeze(2)
        # Reshape from (D,K) to size (1, D, K, 1)
        centersTensor = self.centers.unsqueeze(0).unsqueeze(-1)
        # VLAD core (notation from paper Eq 4).
        descriptors = (
            softAssignments.unsqueeze(1) * (featureTensor - centersTensor)
        ).sum(dim=-1)

        # Intra normalization
        # Descriptors size (1, D, K)
        descriptors = nn.functional.normalize(descriptors, dim=1)

        # L2 normalization
        descriptor = descriptors.view(batch, -1)
        descriptor = nn.functional.normalize(descriptor, dim=1)
        return descriptor


class NetVLAD(nn.Module):
    def __init__(self, pathToMat):
        super(NetVLAD, self).__init__()
        # Define overall network structure.
        vgg16Features = list(models.vgg16().children())[0]
        # Remove last ReLU + MaxPool2d.
        self.backbone = nn.Sequential(*list(vgg16Features.children())[:-2])
        inputDim = 512
        numberOfClusters = 64
        self.vladLayer = VLADLayer(inputDim, numberOfClusters)
        self.whiten = nn.Linear(inputDim * numberOfClusters, 4096)

        # Loading from the costs converted to Python format
        # Take from cvg website https://cvg-data.inf.ethz.ch/hloc/netvlad/Pitts30K_struct.mat
        mat = loadmat(pathToMat, struct_as_record=False, squeeze_me=True)

        for layer, layerFromMat in zip(self.backbone.children(), mat["net"].layers):
            if isinstance(layer, nn.Conv2d):
                # `layer` has shape [output_channels, input_channels, kernel_size, kernel_size]
                # `layerFromMat`has shape [kernel size, kernel size, input channels, output channels]
                # Converting to `layer` format.
                pretrained_weights = torch.from_numpy(layerFromMat.weights[0])
                pretrained_weights = pretrained_weights.permute(3, 2, 0, 1)

                layer.weight = torch.nn.Parameter(pretrained_weights)
                layer.b = torch.from_numpy(layerFromMat.weights[1])

        # Initialize NetVLAD layer
        # NetVLAD weights. Following the reading from cvg guys format
        vladClusterAssociations = mat["net"].layers[30].weights[0]  # D x K
        # Make Pytorch compatible. Dimensions should be K x D x 1
        vladClusterAssociations = (
            torch.tensor(vladClusterAssociations).float().permute([1, 0]).unsqueeze(-1)
        )
        self.vladLayer.clusterAssociations.weight = nn.Parameter(
            vladClusterAssociations
        )

        vladClusterCenters = -mat["net"].layers[30].weights[1]  # D x K
        vladClusterCenters = torch.tensor(vladClusterCenters).float()
        self.vladLayer.centers = nn.Parameter(vladClusterCenters)

        # Whitening weights.
        w = mat["net"].layers[33].weights[0]  # Shape: 1 x 1 x IN x OUT
        b = mat["net"].layers[33].weights[1]  # Shape: OUT
        # Prepare for PyTorch - make sure it is float32 and has right shape
        w = torch.tensor(w).float().squeeze().permute([1, 0])  # OUT x IN
        b = torch.tensor(b.squeeze()).float()  # Shape: OUT
        # Update layer weights.
        self.whiten.weight = nn.Parameter(w)
        self.whiten.bias = nn.Parameter(b)

        # Preprocessing parameters.
        self.preprocess = {
            "mean": mat["net"].meta.normalization.averageImage[0, 0],
            "std": np.array([1, 1, 1], dtype=np.float32),
        }

    def forward(self, image):
        assert image.shape[0] == 3
        image = torch.clamp(image * 255, 0.0, 255.0)  # Input should be 0-255.

        mean_tensor = image.new_tensor(self.preprocess["mean"]).view(1, -1, 1, 1)
        std_tensor = image.new_tensor(self.preprocess["std"]).view(1, -1, 1, 1)

        # Standardize the input image the same way the training dataset was standardized
        image = image - mean_tensor
        image = image / std_tensor

        # Feature extraction.
        # Image shape: (1, 3, 244, 244)
        # Returns a (1, 512, 14, 14) feature volume.
        descriptors = self.backbone(image)
        # Reshape the feature volume -> (1, 512, 14*14)
        batch, channels, width, height = descriptors.size()
        descriptors = descriptors.view(batch, channels, width * height)

        # NetVLAD layer.
        # Pre-normalization.
        descriptors = nn.functional.normalize(descriptors, dim=1)
        descriptor = self.vladLayer(descriptors)

        # Whiten
        descriptor = self.whiten(descriptor)
        descriptor = nn.functional.normalize(descriptor, dim=1)
        return descriptor


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--data_dir",
        type=Path,
        help="Path to the directory with images.",
        required=True,
    )
    parser.add_argument(
        "--output_file", type=Path, help="Output file name, .txt", required=True
    )
    parser.add_argument(
        "--netvlad_weights_file",
        type=Path,
        help="NetVlad weights file. Please download them from https://cvg-data.inf.ethz.ch/hloc/netvlad/Pitts30K_struct.mat to netvlad/data/",
        default="data/Pitts30K_struct.mat",
    )
    args = parser.parse_args()

    # TODO(olga): Make sure it works with GPU too.
    # Get cpu or gpu device for training.
    # device = "cuda" if torch.cuda.is_available() else "cpu"
    device = "cpu"
    print(f"Using {device} device")
    model = NetVLAD(args.netvlad_weights_file).to(device)

    netVladDescriptors = []
    # ToTensor() transforms a numpy array to a correct tensor for network to work with.
    dataset = ImageDataset(img_dir=args.data_dir, transform=ToTensor())
    for image_idx in range(len(dataset)):
        image = dataset[image_idx]
        prediction = model(image)
        pred_np = prediction.detach().numpy()[0]
        netVladDescriptors.append(pred_np)
        print(f"Processed image {image_idx}")

    netVladDescriptors = np.array(netVladDescriptors)
    print("Final descriptors size", netVladDescriptors.shape)
    np.savetxt(args.output_file, netVladDescriptors)


if __name__ == "__main__":
    main()
