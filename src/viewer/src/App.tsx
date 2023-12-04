// Created by O. Vysotska in 2023
import { useState, useEffect } from "react";
import "./App.css";
import { ProtoLoader, MessageType } from "./ProtoLoader";
import { CostMatrix, CostMatrixElement } from "./costMatrix";
import { ImagesLoader } from "./ImagesLoader";
import {
  MatchingResultElement,
  readMatchingResultFromProto,
} from "./matchingResult";
import CostMatrixComponent from "./CostMatrixComponent";

function App() {
  const [costMatrixProto, setCostMatrixProto] = useState(null);
  const [costMatrix, setCostMatrix] = useState<CostMatrix>();
  const [matchingResultProto, setMatchingResultProto] = useState(null);
  const [matchingResult, setMatchingResult] =
    useState<MatchingResultElement[]>();
  const [selectedCostMatrixElement, setSelectedCostMatrixElement] =
    useState<CostMatrixElement>();

  useEffect(() => {
    if (costMatrixProto != null) {
      console.log("Cost Matrix proto is loaded", costMatrixProto);
      setCostMatrix(new CostMatrix(costMatrixProto));
    }
  }, [costMatrixProto]);

  useEffect(() => {
    console.log("Matching result proto changed", matchingResultProto);
    setMatchingResult(readMatchingResultFromProto(matchingResultProto));
  }, [matchingResultProto]);

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
        <div className="costMatrix" style={{ backgroundColor: "ghostwhite" }}>
          <div style={{display: "flex", flexDirection: "column", alignItems: "center"}}>
            <div>
              <ProtoLoader
                onLoad={setCostMatrixProto}
                messageType={MessageType.CostMatrix}
              />
              <ProtoLoader
                onLoad={setMatchingResultProto}
                messageType={MessageType.MatchingResult}
              />
            </div>
          </div>
          {costMatrix && (
            <CostMatrixComponent
              costMatrix={costMatrix}
              matchingResult={matchingResult}
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
            <ImagesLoader
              imageType={"Query"}
              showImageId={selectedCostMatrixElement?.queryId}
            />
          </div>
          <div>
            <ImagesLoader
              imageType={"Reference"}
              showImageId={selectedCostMatrixElement?.refId}
            />
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
