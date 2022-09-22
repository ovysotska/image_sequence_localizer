# Image Sequence Localizer

This project proposes a framework for outdoor **visual place recognition**.

It can recognize visually similar places in GPS-denied environments. 
Thus, can serve as a candidate search technique in the context of full 6 DoF robot pose estimation.

Visual place recognition, also known as "weak localization", is performed here by matching sequences of images.
The assumption is then that the input is a sequence of images or image features respectively.

## Parent project

This repository is a continuation of my previous works [vpr_relocalization](https://github.com/PRBonn/vpr_relocalization) and [online_place_recognition](https://github.com/PRBonn/online_place_recognition).

The plan is to gradually modernize and improve the code by preserving the essential capabilities of the system.

**Essential capabilities**:

1. Given two sequences of image features compute the matching image pairs.
2. Scripts to visualize the results.

## Build

Prerequisites:
```
sudo apt-get install -y libopencv-dev libyaml-cpp-dev libprotobuf-dev libprotoc-dev protobuf-compiler
```
Tested on Ubuntu 20.04.

To build the code:
```
mkdir build
cd build
cmake ..
make -j
```

To be able to use the `python` part, for example for visualization, I recommend setting up a virtual environment of your choice and installing the provided requirements through:
```
pip install -r requirements.txt
```

## Usage

The code is under continuous development but at any point in time you should be able to run the matching procedure through:

``` bash
cd src/python
python run_matching.py \
    --query_features <path_to_features> \
    --reference_features <path_to_features> \
    --dataset_name <dataset_name> \
    --output_dir <path_to_folder>
```
For more details about the parameters, please use `python run_matching.py help`

