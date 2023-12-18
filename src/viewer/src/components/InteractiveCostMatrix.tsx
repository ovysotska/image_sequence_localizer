import React from "react";
import { useState, useRef, useEffect, useCallback } from "react";
import { CostMatrix, CostMatrixElement } from "../resources/costMatrix";
import { ZoomBlockParams } from "./ImageCostMatrix";

import { useElementContext, SelectedElement } from "../context/ElementContext";

import * as d3 from "d3";
import { MatchingResultElement } from "../resources/readers";

type TooltipProps = {
  opacity: number;
  leftCornerPx: number;
  topCornerPx: number;
  queryId: number;
  refId: number;
  value?: number;
};

function Tooltip(props: TooltipProps): React.ReactElement {
  const text =
    "query id: " +
      props.queryId +
      " ref id: " +
      props.refId +
      " value: " +
      props.value?.toFixed(5) ?? "none";
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
  matches?: MatchingResultElement[];
  showMatches?: boolean;
};

function InteractiveCostMatrix(
  props: InteractiveCostMatrixProps
): React.ReactElement {
  const svgRef = useRef<any>();
  const [tooltipVisible, setTooltipVisible] = useState<boolean>(false);
  const [tooltipProps, setTooltipProps] = useState<TooltipProps>();

  const [matchesVisible, setMatchesVisible] = useState<boolean>(false);

  const { globalSelectedElement, setGlobalSelectedElement } =
    useElementContext();

  const height = 500;
  const cellSize = height / props.costMatrix.rows;

  useEffect(() => {
    if (props.showMatches == null) {
      return;
    }
    setMatchesVisible(props.showMatches);
  }, [props.showMatches]);

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
    // Add a group for cost matrix rectangles
    chartGroup
      .append("g")
      .attr("class", "matrixEntries")
      .attr("id", "matrixEntries");
    // Add a group for  matches
    chartGroup.append("g").attr("class", "matches").attr("id", "matches");
  }, [props.costMatrix]);

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

  const moveSliders = useCallback(
    (element: SelectedElement) => {
      const cellInZoomX =
        (element.referenceId - props.zoomBlock.topLeftX) * cellSize +
        cellSize / 2;
      const cellInZoomY =
        (element.queryId - props.zoomBlock.topLeftY) * cellSize + cellSize / 2;

      d3.select(svgRef.current)
        .select(".ySlider")
        .transition()
        .duration(500) // Animation duration in milliseconds
        .attr("y1", cellInZoomY)
        .attr("y2", cellInZoomY);
      d3.select(svgRef.current)
        .select(".xSlider")
        .transition()
        .duration(500) // Animation duration in milliseconds
        .attr("x1", cellInZoomX)
        .attr("x2", cellInZoomX);
    },
    [props.zoomBlock, cellSize]
  );

  // Move sliders when somebody changed the global selected element context
  useEffect(() => {
    if (globalSelectedElement == null) {
      return;
    }
    moveSliders(globalSelectedElement);
  }, [globalSelectedElement, moveSliders]);

  const onCellUnHover = useCallback((event: any) => {
    setTooltipVisible(false);
    event.target.setAttribute("opacity", 1.0);
  }, []);

  const onCellClick = useCallback(
    (event: any, cell: CostMatrixElement) => {
      setGlobalSelectedElement({
        queryId: cell.queryId,
        referenceId: cell.refId,
      });
    },
    [setGlobalSelectedElement]
  );

  useEffect(() => {
    // Retrieve the group for matrix elements
    const valuesGroup = d3.select(svgRef.current).select(".matrixEntries");
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
  }, [
    props.costMatrix,
    props.zoomBlock,
    cellSize,
    onCellHover,
    onCellUnHover,
    onCellClick,
  ]);

  useEffect(() => {
    const valuesGroup = d3.select(svgRef.current).select(".matrixEntries");
    valuesGroup
      .append("line")
      .attr("class", "ySlider")
      .attr("id", "ySlider")
      .attr("x1", 0)
      .attr("y1", 0)
      .attr("x2", height)
      .attr("y2", 0)
      .attr("stroke", "#1e81b0")
      .attr("stroke-width", 2);
    valuesGroup
      .append("line")
      .attr("class", "xSlider")
      .attr("x1", 0)
      .attr("y1", 0)
      .attr("x2", 0)
      .attr("y2", height)
      .attr("stroke", "#eab676")
      .attr("stroke-width", 2);
  }, [props.costMatrix, props.zoomBlock]);

  useEffect(() => {
    if (props.matches == null) {
      return;
    }
    // Retrieve the group for matches
    const matchesGroup = d3.select(svgRef.current).select(".matches");
    matchesGroup
      .selectAll("rect")
      .data(props.matches)
      .enter()
      .append("rect")
      .attr("x", (cell: MatchingResultElement) => {
        return (cell.refId - props.zoomBlock.topLeftX) * cellSize;
      }) // top left corner of the rect
      .attr("y", (cell) => {
        return (cell.queryId - props.zoomBlock.topLeftY) * cellSize;
      })
      .attr("height", cellSize)
      .attr("width", cellSize)
      .attr("fill", (cell: any) => {
        return cell.real
          ? d3.rgb(255, 0, 0).toString()
          : d3.rgb(0, 0, 255).toString();
      })
      .on("mouseover", (event, cell) => {
        onCellHover(event, {
          queryId: cell.queryId,
          refId: cell.refId,
          value: props.costMatrix.at(cell.queryId, cell.refId) ?? -1,
        });
      })
      .on("mouseout", onCellUnHover)
      .on("click", (event, cell) => {
        onCellClick(event, {
          queryId: cell.queryId,
          refId: cell.refId,
          value: props.costMatrix.at(cell.queryId, cell.refId) ?? -1,
        });
      });
  }, [
    props.matches,
    props.costMatrix,
    props.zoomBlock,
    cellSize,
    onCellHover,
    onCellUnHover,
    onCellClick,
  ]);

  useEffect(() => {
    const matchGroup = d3.select(svgRef.current).select(".matches");
    matchGroup.style("visibility", matchesVisible ? "visible" : "hidden");
  }, [matchesVisible, props.matches]);

  return (
    <div
      style={{
        textAlign: "center",
      }}
    >
      <div>
        <h3> Interactive Zoom in</h3>
      </div>
      <div>
        <svg
          ref={svgRef}
          style={{
            backgroundColor: "lightskyblue",
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
