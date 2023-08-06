import React, { forwardRef } from "react";
import { useState, useRef, useEffect } from "react";

const kZoomWindowPx = 30;

type ZoomBlockParams = {
  topLeftX: number; // column
  topLeftY: number; // row
  windowWidthPx: number;
  windowHeightPx: number;
};

type ZoomTooltipProps = {
  coordX?: string;
  coordY?: string;
};

const ZoomTooltip = forwardRef((props: ZoomTooltipProps, pixelZoomRef: any) => {
  useEffect(() => {
    if (pixelZoomRef.current != null) {
      pixelZoomRef.current.width = 300;
      pixelZoomRef.current.height = 300;

      const zoomContext = pixelZoomRef.current.getContext("2d");
      if (zoomContext != null) {
        zoomContext.imageSmoothingEnabled = false;
      }
    }
  }, [pixelZoomRef]);

  return (
    <div
      className="ZoomView"
      style={{
        margin: "10px",
        position: "absolute",
        top: props.coordY + "px",
        left: props.coordX + "px",
        border: "3px solid pink",
        visibility: props.coordX && props.coordY ? "visible" : "hidden",
      }}
    >
      <canvas ref={pixelZoomRef}></canvas>
    </div>
  );
});

function drawZoomBoxInContext(
  imageCanvas: HTMLCanvasElement,
  pixelZoomCanvas: HTMLCanvasElement,
  zoomBbox: ZoomBlockParams
) {
  if (imageCanvas == null || pixelZoomCanvas == null) {
    return;
  }

  if (pixelZoomCanvas != null) {
    const zoomContext = pixelZoomCanvas.getContext("2d");
    if (zoomContext != null) {
      zoomContext.drawImage(
        imageCanvas,
        zoomBbox.topLeftX,
        zoomBbox.topLeftY,
        zoomBbox.windowWidthPx,
        zoomBbox.windowHeightPx,
        0,
        0,
        pixelZoomCanvas.width,
        pixelZoomCanvas.height
      );
    }
  }
}

type ImageCostMatrixProps = {
  image: ImageBitmap;
  setZoomParams: (zoomParams: ZoomBlockParams) => void;
};

type ImageSize = {
  width: number;
  height: number;
};
type HoverCoords = {
  screenX: number;
  screenY: number;
};

function ImageCostMatrix(props: ImageCostMatrixProps): React.ReactElement {
  const imageCanvasRef = useRef<HTMLCanvasElement>(null);
  const pixelZoomRef = useRef<HTMLCanvasElement>(null);
  const [imageSize, setImageSize] = useState<ImageSize>({
    width: 0,
    height: 0,
  });
  const [hoverCoords, setHoverCoords] = useState<HoverCoords>();

  // Assumptions: Canvas size is adapted to image size.
  // Zoom image is always rescaled to 300px x 300px
  useEffect(() => {
    // Default for Firefox area:
    // canvasRef.current.width = 300;
    // canvasRef.current.height = 150;

    if (props.image == null) {
      console.error("Image is empty.");
      return;
    }

    const imageCanvas = imageCanvasRef.current;
    if (imageCanvas == null) {
      console.error("Canvas is not set");
      return;
    }

    const imageContext = imageCanvas.getContext("2d");
    if (imageContext == null) {
      console.error("Image Context is null");
      return;
    }

    console.log("Image size", props.image.width, props.image.height);
    setImageSize({ width: props.image.width, height: props.image.height });
    imageCanvas.width = props.image.width;
    imageCanvas.height = props.image.height;

    // Draw image of exact size.
    imageContext.drawImage(props.image, 0, 0);
  }, [props.image]);

  function onHoverOverImage(event: any) {
    const xCenterPx = event.nativeEvent.layerX;
    const yCenterPx = event.nativeEvent.layerY;

    setHoverCoords({ screenX: event.clientX, screenY: event.clientY });

    if (pixelZoomRef.current != null && imageCanvasRef.current != null) {
      // const canvas = pixelZoomRef.current;
      const zoomBbox: ZoomBlockParams = {
        topLeftX: Math.min(
          Math.max(0, xCenterPx - kZoomWindowPx / 2),
          imageSize.width - kZoomWindowPx
        ),
        topLeftY: Math.min(
          Math.max(0, yCenterPx - kZoomWindowPx / 2),
          imageSize.height - kZoomWindowPx
        ),
        windowWidthPx: kZoomWindowPx,
        windowHeightPx: kZoomWindowPx,
      };

      drawZoomBoxInContext(
        imageCanvasRef.current,
        pixelZoomRef.current,
        zoomBbox
      );
    }
  }

  function onClick(event: any) {
    const x = event.nativeEvent.layerX;
    const y = event.nativeEvent.layerY;
    const topLeftX = Math.floor(Math.max(0, x - kZoomWindowPx / 2));
    const topLeftY = Math.floor(Math.max(0, y - kZoomWindowPx / 2));
    props.setZoomParams({
      topLeftX: topLeftX,
      topLeftY: topLeftY,
      windowWidthPx: kZoomWindowPx,
      windowHeightPx: kZoomWindowPx,
    });
  }

  return (
    <div>
      <div
        className="costMatrix"
        style={{
          maxHeight: "500px",
          overflow: "auto",
          margin: "10px",
        }}
      >
        <canvas
          ref={imageCanvasRef}
          onMouseMove={onHoverOverImage}
          onMouseLeave={() => {
            setHoverCoords(undefined);
          }}
          onClick={onClick}
        />
      </div>
      {hoverCoords && (
        <ZoomTooltip
          coordX={hoverCoords.screenX.toFixed(0)}
          coordY={hoverCoords.screenY.toFixed(0)}
          ref={pixelZoomRef}
        ></ZoomTooltip>
      )}
    </div>
  );
}

export { ImageCostMatrix };
export type { ImageCostMatrixProps, ZoomBlockParams };
