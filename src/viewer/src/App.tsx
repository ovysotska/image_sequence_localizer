// Created by O. Vysotska in 2023
import { useState } from "react";
import "./App.css";

import CostMatrixComponent from "./components/CostMatrixComponent";
import { ImagePreviewComponent } from "./components/ImagePreviewComponent";
import DataLoader from "./components/DataLoader";

import { ElementProvider } from "./context/ElementContext";

function App() {
  const [costMatrixProtoFile, setCostMatrixProtoFile] = useState<File>();
  const [matchingResultProtoFile, setMatchingResultProtoFile] =
    useState<File>();
  const [queryImageFiles, setQueryImageFiles] = useState<File[]>();
  const [referenceImageFiles, setReferenceImageFiles] = useState<File[]>();

  return (
    <ElementProvider>
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
          <div
            className="costMatrix"
            style={{ margin: "0 10px 0 10px", backgroundColor: "alicewhite" }}
          >
            {costMatrixProtoFile && (
              <CostMatrixComponent
                costMatrixProtoFile={costMatrixProtoFile}
                matchingResultProtoFile={matchingResultProtoFile}
              />
            )}
          </div>
          <ImagePreviewComponent
            queryImageFiles={queryImageFiles}
            referenceImageFiles={referenceImageFiles}
          />
        </div>
      </div>
    </ElementProvider>
  );
}

export default App;
