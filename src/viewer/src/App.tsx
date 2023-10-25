// Created by O. Vysotska in 2023
import { useState, useEffect } from "react";
import "./App.css";
import { ImageCostMatrix, ZoomBlockParams } from "./ImageCostMatrix";
import { ProtoLoader, MessageType } from "./ProtoLoader";
import { CostMatrix, CostMatrixElement } from "./costMatrix";
import InteractiveCostMatrix from "./InteractiveCostMatrix";
import { ImagesLoader } from "./ImagesLoader";
import {
  MatchingResultElement,
  readMatchingResultFromProto,
} from "./matchingResult";

function App() {
  const [image, setImage] = useState<ImageBitmap>();
  const [costMatrixProto, setCostMatrixProto] = useState(null);
  const [matchingResultProto, setMatchingResultProto] = useState(null);
  const [matchingResult, setMatchingResult] =
    useState<MatchingResultElement[]>();
  const [costMatrix, setCostMatrix] = useState<CostMatrix>();
  const [zoomParams, setZoomParams] = useState<ZoomBlockParams>();
  const [zoomedCostMatrix, setZoomedCostMatrix] = useState<CostMatrix>();
  const [selectedCostMatrixElement, setSelectedCostMatrixElement] =
    useState<CostMatrixElement>();

  useEffect(() => {
    if (costMatrixProto != null) {
      console.log("Cost Matrix proto is loaded", costMatrixProto);
      setCostMatrix(new CostMatrix(costMatrixProto));
    }
  }, [costMatrixProto]);

  useEffect(() => {
    if (costMatrix != null) {
      costMatrix.createImage().then((result) => {
        setImage(result);
      });
    }
  }, [costMatrix]);

  useEffect(() => {
    if (zoomParams == null || costMatrix == null) {
      return;
    }
    setZoomedCostMatrix(
      costMatrix.getSubMatrix(
        zoomParams.topLeftX,
        zoomParams.topLeftY,
        zoomParams.windowHeightPx
      )
    );
  }, [zoomParams, costMatrix]);

  useEffect(() => {
    console.log("Matching result changed", matchingResultProto);
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
          <ProtoLoader
            onLoad={setCostMatrixProto}
            messageType={MessageType.CostMatrix}
          />
          <ProtoLoader
            onLoad={setMatchingResultProto}
            messageType={MessageType.MatchingResult}
          />
          <div
            style={{
              display: "flex",
              flexDirection: "row",
              flexWrap: "wrap",
              justifyContent: "center",
            }}
          >
            <div>
              {image && (
                <ImageCostMatrix
                  image={image}
                  setZoomParams={setZoomParams}
                  width="100%"
                  height="100%"
                  matches={matchingResult}
                />
              )}
            </div>
            <div>
              {zoomParams && zoomedCostMatrix && (
                <InteractiveCostMatrix
                  costMatrix={zoomedCostMatrix}
                  zoomBlock={zoomParams}
                  setSelectedElement={setSelectedCostMatrixElement}
                />
              )}
            </div>
          </div>
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
