// Created by O. Vysotska in 2023
import { useState } from "react";
import "./App.css";
import { CostMatrixElement } from "./resources/costMatrix";
import { ImagesPreview } from "./components/ImagesPreview";
import CostMatrixComponent from "./components/CostMatrixComponent";

import DataLoader from "./components/DataLoader";

function App() {
  const [costMatrixProtoFile, setCostMatrixProtoFile] = useState<File>();
  const [matchingResultProtoFile, setMatchingResultProtoFile] =
    useState<File>();
  const [queryImageFiles, setQueryImageFiles] = useState<File[]>();
  const [referenceImageFiles, setReferenceImageFiles] = useState<File[]>();

  const [selectedCostMatrixElement, setSelectedCostMatrixElement] =
    useState<CostMatrixElement>();

  return (
    <div className="App">
      <div
        style={{
          display: "flex",
          flexDirection: "column",
          flexWrap: "wrap",
          justifyContent: "center",
          gap: "10px",
        }}
      >
        <h1 style={{ textAlign: "center" }}>Cost Matrix Viewer</h1>
        <DataLoader
          setCostMatrixProtoFile={setCostMatrixProtoFile}
          setMatchingResultProtoFile={setMatchingResultProtoFile}
          setQueryImageFiles={setQueryImageFiles}
          setReferenceImageFiles={setReferenceImageFiles}
        />
        <div className="costMatrix" style={{ backgroundColor: "ghostwhite" }}>
          {costMatrixProtoFile && (
            <CostMatrixComponent
              costMatrixProtoFile={costMatrixProtoFile}
              matchingResultProtoFile={matchingResultProtoFile}
              setSelectedCostMatrixElement={setSelectedCostMatrixElement}
            />
          )}
        </div>

        <div
          className="imageLoaders"
          style={{
            backgroundColor: "lavender",
            display: "flex",
            flexDirection: "row",
            flexWrap: "wrap",
            justifyContent: "center",
            columnGap: "20px",
          }}
        >
          <div>
            <ImagesPreview
              imageFiles={queryImageFiles}
              imageSource={"Query"}
              showImageId={selectedCostMatrixElement?.queryId}
            />
          </div>
          <div>
            <ImagesPreview
              imageFiles={referenceImageFiles}
              imageSource={"Reference"}
              showImageId={selectedCostMatrixElement?.refId}
            />
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
