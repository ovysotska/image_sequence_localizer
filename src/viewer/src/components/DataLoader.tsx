type DataLoaderProps = {
  setMatchingResultProtoFile: (file: File) => void;
  setCostMatrixProtoFile: (file: File) => void;
  setQueryImageFiles: (files: File[]) => void;
  setReferenceImageFiles: (file: File[]) => void;
};

function DataLoader(props: DataLoaderProps): React.ReactElement {
  async function onChange(event: React.ChangeEvent<HTMLInputElement>) {
    const fileList = event.target.files;
    if (fileList == null) {
      return;
    }
    const files = Array.from(fileList);

    const costMatrixProtoFile = files.find((file) => {
      return file.webkitRelativePath.split("/")[1].endsWith(".CostMatrix.pb");
    });
    if (costMatrixProtoFile == null) {
      console.warn("CostMatrix proto file was not found");
    } else {
      props.setCostMatrixProtoFile(costMatrixProtoFile);
    }

    const matchingResultProtoFile = files.find((file) => {
      return file.webkitRelativePath
        .split("/")[1]
        .endsWith(".MatchingResult.pb");
    });

    if (matchingResultProtoFile == null) {
      console.warn("MatchingResult proto file was not found");
    } else {
      props.setMatchingResultProtoFile(matchingResultProtoFile);
    }

    const queryImageFiles = files.filter((file) => {
      console.log(file.webkitRelativePath.split("/")[1]);
      return file.webkitRelativePath.split("/")[1] === "query_images";
    });
    if (queryImageFiles.length === 0) {
      console.warn("Query images was not found");
    } else {
      props.setQueryImageFiles(queryImageFiles);
    }

    const referenceImageFiles = files.filter((file) => {
      return file.webkitRelativePath.split("/")[1] === "reference_images";
    });
    if (referenceImageFiles.length === 0) {
      console.warn("Reference images was not found");
    } else {
      props.setReferenceImageFiles(referenceImageFiles);
    }
  }

  return (
    <div style={{ textAlign: "center" }}>
      <label htmlFor="folderPicker">Select folder </label>
      <input
        type="file"
        id="folderPicker"
        name="fileList"
        /* @ts-expect-error */
        webkitdirectory=""
        directory=""
        multiple
        onChange={onChange}
      />
    </div>
  );
}

export default DataLoader;
