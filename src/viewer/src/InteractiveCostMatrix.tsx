import React from "react";
import { useState, useRef, useEffect, useCallback } from "react";
import CostMatrix, { CostMatrixElement } from "./costMatrix";
import { ZoomBlockParams } from "./ImageCostMatrix";

import * as d3 from "d3";

type TooltipProps = {
  opacity: number;
  leftCornerPx: number;
  topCornerPx: number;
  queryId: number;
  refId: number;
  value: number;
};

function Tooltip(props: TooltipProps): React.ReactElement {
  const text =
    "query id: " +
    props.queryId +
    " ref id: " +
    props.refId +
    " value: " +
    props.value.toFixed(5);
  return (
    <div
      title={"tooltip"}
      style={{
        opacity: props.opacity,
        position: "absolute",
        left: props.leftCornerPx + "px",
        top: props.topCornerPx + "px",
        background: "white",
        inlineSize: "110px",
        overflowWrap: "break-word",
        textAlign: "left",
        // visibility: "hidden",
        // transition: 50,
      }}
    >
      {text}
    </div>
  );
}

type InteractiveCostMatrixProps = {
  costMatrix: CostMatrix;
  zoomBlock: ZoomBlockParams;
};

function InteractiveCostMatrix(
  props: InteractiveCostMatrixProps
): React.ReactElement {
  const svgRef = useRef<any>();
  const [tooltipVisible, setTooltipVisible] = useState<boolean>(false);
  const [tooltipProps, setTooltipProps] = useState<TooltipProps>();

  const [selectedPixel, setSelectedPixel] = useState<CostMatrixElement>();

  const height = 500;
  const cellSize = height / props.costMatrix.rows;

  const onCellHover = useCallback((event: any, cell: CostMatrixElement) => {
    setTooltipVisible(true);
    event.target.setAttribute("opacity", 0.5);
    setTooltipProps({
      leftCornerPx: event.layerX + 10,
      topCornerPx: event.layerY + 10,
      queryId: cell.queryId,
      refId: cell.refId,
      value: cell.value,
      opacity: 1.0,
    });
  }, []);

  const onCellUnHover = useCallback((event: any) => {
    setTooltipVisible(false);
    event.target.setAttribute("opacity", 1.0);
  }, []);

  const onCellClick = useCallback((event: any, cell: CostMatrixElement) => {
    setSelectedPixel(cell);
  }, []);

  useEffect(() => {
    console.log("Changed the value of selectedPixel", selectedPixel);
  }, [selectedPixel]);

  useEffect(() => {
    if (svgRef.current == null) {
      console.log("Svg ref is not set");
      return;
    }
    const svgElement = d3.select(svgRef.current);
    if (!svgElement.select("#main").empty()) {
      console.log("Removing previous chart");
      svgElement.select("#main").remove();
    }

    const chartGroup = svgElement
      .append("g")
      .attr("id", "main")
      .attr("class", "main");

    const valuesGroup = chartGroup.append("g").attr("class", "values");

    valuesGroup
      .selectAll("rect")
      .data(props.costMatrix.valuesArray)
      .enter()
      .append("rect")
      .attr("x", (cell: CostMatrixElement) => {
        return (cell.refId - props.zoomBlock.topLeftX) * cellSize;
      }) // top left corner of the rect
      .attr("y", (cell) => {
        return (cell.queryId - props.zoomBlock.topLeftY) * cellSize;
      })
      .attr("height", cellSize)
      .attr("width", cellSize)
      .attr("fill", (cell: any) => {
        const color = 255 * cell.value;
        return d3.rgb(color, color, color).toString();
      })
      .on("mouseover", (event, cell) => {
        onCellHover(event, cell);
      })
      .on("mouseout", onCellUnHover)
      .on("click", onCellClick);
  }, [props, cellSize, onCellHover, onCellUnHover, onCellClick]);

  return (
    <div
      style={{
        display: "flex",
        flexDirection: "column",
        justifyContent: "center",
      }}
    >
      <div>
        <h3> Interactive Zoom in</h3>
      </div>
      <div>
        <svg
          ref={svgRef}
          style={{
            backgroundColor: "salmon",
            width: "400px",
            height: "400px",
            margin: "10px",
          }}
        ></svg>
      </div>
      {tooltipVisible && tooltipProps && <Tooltip {...tooltipProps} />}
    </div>
  );
}

export default InteractiveCostMatrix;
