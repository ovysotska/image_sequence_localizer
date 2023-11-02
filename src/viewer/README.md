# Viewer
This viewer should be used for debug purposes. In the viewer, you can:
* Examine cost matrices that are stored in ".CostMatrix.pb" format.
* Preview images in both sequences.
* OnClick on the cost matrix preview the corresponding image pair.
* Preview the matchign result, stored in ".MatchingResult.pb".

## Install
Make sure you have installed **npm** and **node** of the correct version.
Use **nvm** to install them. For details, check here https://github.com/nvm-sh/nvm

Install nvm
```bash
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash
``` 

Install **node** at curently tested version:
```bash
nvm install 20.9
```

Upgrade **npm** respectively:
```bash
npm update
```
Check the version:
```bash
npm --version
```
if you see `10.1.0` you are good to go.
Copy the proto definitions to be able to use them in the viewer.
```bash
./copy_protos.sh
```
Install all the necessary nodes with proper versions through:
```bash
npm install
```


## Use
To run the viewer, call 
```bash
npm run start
```
