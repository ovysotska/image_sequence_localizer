// Created by O. Vysotska in 2023
import { useState, useEffect } from "react";
import { CostMatrix, CostMatrixElement } from "./costMatrix";
import { ImageCostMatrix, ZoomBlockParams } from "./ImageCostMatrix";
import { MatchingResultElement } from "./matchingResult";
import InteractiveCostMatrix from "./InteractiveCostMatrix";

type CostMatrixProps = {
  costMatrix: CostMatrix;
  matchingResult?: MatchingResultElement[];
  setSelectedCostMatrixElement: (element: CostMatrixElement) => void;
};

function CostMatrixComponent(props: CostMatrixProps): React.ReactElement {
  const [image, setImage] = useState<ImageBitmap>();
  const [matchingResult, setMatchingResult] =
    useState<MatchingResultElement[]>();
  const [zoomParams, setZoomParams] = useState<ZoomBlockParams>();
  const [zoomedCostMatrix, setZoomedCostMatrix] = useState<CostMatrix>();
  const [matchingResultVisible, setMatchingResultVisible] =
    useState<boolean>(false);

  useEffect(() => {
    if (props.costMatrix != null) {
      props.costMatrix.createImage().then((result) => {
        setImage(result);
      });
    }
  }, [props.costMatrix]);

  useEffect(() => {
    setMatchingResult(props.matchingResult);
  }, [props.matchingResult]);

  useEffect(() => {
    if (zoomParams == null || props.costMatrix == null) {
      return;
    }
    setZoomedCostMatrix(
      props.costMatrix.getSubMatrix(
        zoomParams.topLeftX,
        zoomParams.topLeftY,
        zoomParams.windowHeightPx
      )
    );
  }, [zoomParams, props.costMatrix]);

  function showMatchingResult(event: any) {
    if (event.target.checked) {
      setMatchingResultVisible(true);
    } else {
      setMatchingResultVisible(false);
    }
  }

  return (
    <div
      style={{
        display: "flex",
        flexDirection: "column",
      }}
    >
      <div>
        <input id="checkbox" type="checkbox" onChange={showMatchingResult} />
        <label htmlFor="checkbox">Show matching result</label>
      </div>
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
              showMatches={matchingResultVisible}
            />
          )}
        </div>
        <div>
          {zoomParams && zoomedCostMatrix && (
            <InteractiveCostMatrix
              costMatrix={zoomedCostMatrix}
              zoomBlock={zoomParams}
              setSelectedElement={props.setSelectedCostMatrixElement}
              showMatches={matchingResultVisible}
              matches={matchingResult}
            />
          )}
        </div>
      </div>
    </div>
  );
}

export default CostMatrixComponent;
