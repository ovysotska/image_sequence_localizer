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
      return file.webkitRelativePath.split("/")[1] === "query_images";
    });
    console.log("Query images", queryImageFiles);
    if (queryImageFiles.length === 0) {
      console.warn("Query images are not found");
      props.setQueryImageFiles([]);
    } else {
      props.setQueryImageFiles(queryImageFiles);
    }

    const referenceImageFiles = files.filter((file) => {
      return file.webkitRelativePath.split("/")[1] === "reference_images";
    });
    if (referenceImageFiles.length === 0) {
      console.warn("Reference images are not found");
      props.setReferenceImageFiles([]);
    } else {
      props.setReferenceImageFiles(referenceImageFiles);
    }
  }

  return (
    <div
      style={{
        textAlign: "center",
        boxShadow: "0 0 15px 4px rgba(0,0,0,0.06)",
        padding: "10px",
        margin: "10px",
      }}
    >
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
