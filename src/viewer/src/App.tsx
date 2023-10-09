// Created by O. Vysotska in 2023
import { useState, useEffect } from "react";
import "./App.css";
import { ImageCostMatrix, ZoomBlockParams } from "./ImageCostMatrix";
import ProtoLoader from "./ProtoLoader";
import { CostMatrix, CostMatrixElement } from "./costMatrix";
import InteractiveCostMatrix from "./InteractiveCostMatrix";
import { ImagesLoader } from "./ImagesLoader";

function App() {
  const [image, setImage] = useState<ImageBitmap>();
  const [costMatrixProto, setCostMatrixProto] = useState(null);
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

  return (
    <div className="App">
      <div style={{ width: "80%", margin: "auto" }}>
        <h1 style={{ textAlign: "center" }}>Cost Matrix Viewer</h1>
        <div className="costMatrix" style={{ backgroundColor: "ghostwhite" }}>
          <ProtoLoader onLoad={setCostMatrixProto} />
          <div
            style={{
              display: "flex",
              flexDirection: "row",
              justifyContent: "left",
            }}
          >
            <div>
              {image && (
                <ImageCostMatrix image={image} setZoomParams={setZoomParams} />
              )}
            </div>
            <div style={{ margin: "auto" }}>
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
            justifyContent: "center",
          }}
        >
          <div style={{ width: "50%", alignContent: "center" }}>
            <ImagesLoader
              imageType={"Query"}
              showImageId={selectedCostMatrixElement?.queryId}
            />
          </div>
          <div style={{ width: "50%" }}>
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
