import React from "react";
import { useState, useEffect } from "react";
import "./App.css";
import { ImageCostMatrix, ZoomBlockParams } from "./ImageCostMatrix";
import ProtoLoader from "./ProtoLoader";
import CostMatrix from "./costMatrix";
import InteractiveCostMatrix from "./InteractiveCostMatrix";

function App() {
  const [image, setImage] = useState<ImageBitmap>();
  const [costMatrixProto, setCostMatrixProto] = useState(null);
  const [costMatrix, setCostMatrix] = useState<CostMatrix>();
  const [zoomParams, setZoomParams] = useState<ZoomBlockParams>();
  const [zoomedCostMatrix, setZoomedCostMatrix] = useState<CostMatrix>();

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

  return (
    <div className="App">
      <h1 style={{ textAlign: "center" }}>Cost Matrix Viewer</h1>
      <ProtoLoader onLoad={setCostMatrixProto} />

      <div
        style={{
          display: "flex",
          flexDirection: "row",
        }}
      >
        <div>
          {image && (
            <ImageCostMatrix image={image} setZoomParams={setZoomParams} />
          )}
        </div>
        <div>
          {zoomParams && zoomedCostMatrix && (
            <InteractiveCostMatrix
              costMatrix={zoomedCostMatrix}
              zoomBlock={zoomParams}
            />
          )}
        </div>
      </div>
    </div>
  );
}

export default App;
