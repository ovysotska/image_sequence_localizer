// Created by O. Vysotska in 2023
import { useState, useEffect } from "react";
import { CostMatrix } from "../resources/costMatrix";
import { ImageCostMatrix, ZoomBlockParams } from "./ImageCostMatrix";
import InteractiveCostMatrix from "./InteractiveCostMatrix";

import { FormGroup, FormControlLabel, Switch } from "@mui/material";

import {
  readMatchingResultFromProto,
  readProtoFromFile,
  MatchingResultElement,
  ProtoMessageType,
} from "../resources/readers";

function getMatchingResultInZoomBlock(
  results: MatchingResultElement[],
  zoomParams: ZoomBlockParams
) {
  return results.filter(
    (result) =>
      result.queryId >= zoomParams.topLeftY &&
      result.queryId < zoomParams.topLeftY + zoomParams.windowHeightPx &&
      result.refId >= zoomParams.topLeftX &&
      result.refId < zoomParams.topLeftX + zoomParams.windowWidthPx
  );
}

type CostMatrixProps = {
  costMatrixProtoFile: File;
  matchingResultProtoFile?: File;
  matchingCostsProtoFile?: File;
};

function CostMatrixComponent(props: CostMatrixProps): React.ReactElement {
  const [costMatrix, setCostMatrix] = useState<CostMatrix>();
  const [image, setImage] = useState<ImageBitmap>();
  const [matchingResult, setMatchingResult] =
    useState<MatchingResultElement[]>();
  // This should be some useful data structure to propagate the expanded costs.
  // For now just checking that this coud be loaded.
  const [matchingCostsProto,  setMatchingCostsProto] = useState<any>();
  const [zoomParams, setZoomParams] = useState<ZoomBlockParams>();
  const [zoomedCostMatrix, setZoomedCostMatrix] = useState<CostMatrix>();
  const [matchingResultVisible, setMatchingResultVisible] =
    useState<boolean>(false);

  // Read costMatrix from proto file.
  useEffect(() => {
    if (props.costMatrixProtoFile == null) {
      return;
    }
    readProtoFromFile(props.costMatrixProtoFile, ProtoMessageType.CostMatrix)
      .then((costMatrixProto) => {
        setCostMatrix(new CostMatrix(costMatrixProto));
      })
      .catch((e) => {
        console.log("Couldn't read file", props.costMatrixProtoFile);
      });
  }, [props.costMatrixProtoFile]);

  useEffect(() => {
    if (costMatrix != null) {
      costMatrix.createImage().then((result) => {
        setImage(result);
      });
    }
  }, [costMatrix]);

  // Read Matching result proto file
  useEffect(() => {
    if (props.matchingResultProtoFile == null) {
      return;
    }
    readProtoFromFile(
      props.matchingResultProtoFile,
      ProtoMessageType.MatchingResult
    )
      .then((matchingResultProto) => {
        setMatchingResult(readMatchingResultFromProto(matchingResultProto));
      })
      .catch((e) => {
        console.log("Couldn't read file", props.matchingResultProtoFile);
      });
  }, [props.matchingResultProtoFile]);


  // Read matching costs -> for expanded costs proto file
  useEffect(() => {
    if (props.matchingCostsProtoFile == null) {
      return;
    }
    readProtoFromFile(
      props.matchingCostsProtoFile,
      ProtoMessageType.MatchingCosts
    )
      .then((matchingCostsProto) => {
        setMatchingCostsProto(matchingCostsProto);
      })
      .catch((e) => {
        console.log("Couldn't read file", props.matchingCostsProtoFile);
      });
  }, [props.matchingCostsProtoFile]);

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
      <FormGroup
        style={{
          display: "flex",
          flexDirection: "row",
          justifyContent: "center",
        }}
      >
        <FormControlLabel
          disabled={matchingResult ? false : true}
          control={<Switch onChange={showMatchingResult} color="primary" />}
          label="Matching result"
        />
        <FormControlLabel
          disabled
          control={<Switch onChange={showMatchingResult} />}
          label="Expanded nodes"
        />
      </FormGroup>
      <div
        style={{
          display: "flex",
          flexDirection: "row",
          flexWrap: "wrap",
          justifyContent: "center",
          backgroundColor: "ghostwhite"
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
              showMatches={matchingResultVisible}
              matches={
                matchingResult &&
                getMatchingResultInZoomBlock(matchingResult, zoomParams)
              }
            />
          )}
        </div>
      </div>
    </div>
  );
}

export default CostMatrixComponent;
