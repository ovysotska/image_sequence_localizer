import React from "react";
import { useState, useRef, useEffect } from "react";

const kZoomWindowPx = 25;

type ZoomBlockParams = {
  topLeftX: number; // column
  topLeftY: number; // row
  windowWidthPx: number;
  windowHeightPx: number;
};

type ImageCostMatrixProps = {
  image: ImageBitmap;
  setZoomParams: (zoomParams: ZoomBlockParams) => void;
};

type ImageSize = {
  width: number;
  height: number;
};

function ImageCostMatrix(props: ImageCostMatrixProps): React.ReactElement {
  const imageCanvasRef = useRef<HTMLCanvasElement>(null);
  const pixelZoomRef = useRef<HTMLCanvasElement>(null);
  const [imageSize, setImageSize] = useState<ImageSize>({
    width: 0,
    height: 0,
  });

  function onHoverOverImage(event: any) {
    let x, y;
    x = event.nativeEvent.layerX;
    y = event.nativeEvent.layerY;

    if (pixelZoomRef.current != null) {
      const canvas = pixelZoomRef.current;
      const context = canvas.getContext("2d");
      if (context != null) {
        drawZoomBoxInContext(context, x, y);
      }
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

  // Assumptions: Canvas size is adapted to image size.
  // Zoom image is always rescaled to 300px x 300px
  useEffect(() => {
    // Default for Firefox area:
    // canvasRef.current.width = 300;
    // canvasRef.current.height = 150;
    if (pixelZoomRef.current == null) {
      return;
    }

    pixelZoomRef.current.width = 300;
    pixelZoomRef.current.height = 300;

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

    const zoomContext = pixelZoomRef.current.getContext("2d");
    if (zoomContext != null) {
      zoomContext.imageSmoothingEnabled = false;
    }
  }, [props.image]);

  const drawZoomBoxInContext = (
    context: CanvasRenderingContext2D,
    xCenterPx: number,
    yCenterPx: number
  ) => {
    if (imageSize == null) {
      return;
    }
    const zoomBbox = {
      xStart: Math.min(
        Math.max(0, xCenterPx - kZoomWindowPx / 2),
        imageSize.width - kZoomWindowPx
      ),
      yStart: Math.min(
        Math.max(0, yCenterPx - kZoomWindowPx / 2),
        imageSize.height - kZoomWindowPx
      ),
      width: kZoomWindowPx,
      heigt: kZoomWindowPx,
    };

    if (imageCanvasRef.current == null || pixelZoomRef.current == null) {
      return;
    }

    context.drawImage(
      imageCanvasRef.current,
      zoomBbox.xStart,
      zoomBbox.yStart,
      zoomBbox.width,
      zoomBbox.heigt,
      0,
      0,
      pixelZoomRef.current.width,
      pixelZoomRef.current.height
    );
  };

  return (
    <div
      style={{
        display: "flex",
        flexDirection: "row",
        gap: "10px",
        justifyContent: "center",
        backgroundColor: "ghostwhite",
      }}
    >
      <div
        className="costMatrix"
        style={{
          maxWidth: "50%",
          maxHeight: "500px",
          overflow: "auto",
          margin: "10px",
        }}
      >
        <canvas
          ref={imageCanvasRef}
          onMouseMove={onHoverOverImage}
          onClick={onClick}
        />
      </div>

      <div
        className="ZoomView"
        style={{
          display: "flex",
          flexDirection: "column",
          width: "40%",
          margin: "10px",
          maxWidth: "400px",
        }}
      >
        <h3 style={{ textAlign: "center" }}> Zoom In View</h3>
        <canvas ref={pixelZoomRef}></canvas>
      </div>
    </div>
  );
}

export { ImageCostMatrix };
export type { ImageCostMatrixProps, ZoomBlockParams };
