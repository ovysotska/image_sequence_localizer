import React, { forwardRef } from "react";
import { useState, useRef, useEffect } from "react";
import { MatchingResultElement } from "./matchingResult";

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
        zIndex: 2
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

function overlayMatchingResult(
  imageData: Uint8ClampedArray,
  imageWidth: number,
  matchingResult?: MatchingResultElement[]
) {
  if (matchingResult == null) {
    return;
  }
  matchingResult.forEach((element) => {
    const pixelIdx = (element.queryId * imageWidth + element.refId) * 4;
    if (pixelIdx < 0 || pixelIdx + 4 >= imageData.length) {
      console.log("Error: Estimated image element is outside bounds");
      return;
    }
    if (element.real) {
      // Assign red pixel
      imageData[pixelIdx] = 255; // red
      imageData[pixelIdx + 1] = 0; // green
      imageData[pixelIdx + 2] = 0; // blue
      imageData[pixelIdx + 3] = 255; // alpha
    } else {
      // Assign blue pixel
      imageData[pixelIdx] = 0; // red
      imageData[pixelIdx + 1] = 0; // green
      imageData[pixelIdx + 2] = 255; // blue
      imageData[pixelIdx + 3] = 255; // alpha
    }
  });
}

type ImageCostMatrixProps = {
  image: ImageBitmap;
  setZoomParams: (zoomParams: ZoomBlockParams) => void;
  width: string;
  height: string;
  matches?: MatchingResultElement[];
  showMatches?: boolean;
};

type ImageSize = {
  width: number;
  height: number;
};
type HoverCoords = {
  screenX: number;
  screenY: number;
};

// TODO(olga): Maybe separate canvas for image and matching path is a better solution. Then need to make sure it gets propagated to the zoomTooltip

function ImageCostMatrix(props: ImageCostMatrixProps): React.ReactElement {
  const imageCanvasRef = useRef<HTMLCanvasElement>(null);
  const pixelZoomRef = useRef<HTMLCanvasElement>(null);
  const [image, setImage] = useState<ImageBitmap>();
  const [imageSize, setImageSize] = useState<ImageSize>({
    width: 0,
    height: 0,
  });

  const [hoverCoords, setHoverCoords] = useState<HoverCoords>();

  useEffect(() => {
    if (props.image == null) {
      console.log("Image is empty");
      return;
    }
    setImage(props.image);
  }, [props.image]);

  // Assumptions: Canvas size is adapted to image size.
  // Zoom image is always rescaled to 300px x 300px
  useEffect(() => {
    // Default for Firefox area:
    // canvasRef.current.width = 300;
    // canvasRef.current.height = 150;

    if (image == null) {
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

    console.log("Image size", image.width, image.height);
    setImageSize({ width: image.width, height: image.height });
    imageCanvas.width = image.width;
    imageCanvas.height = image.height;

    // Draw image of exact size.
    imageContext.drawImage(image, 0, 0);
  }, [image]);

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

  useEffect(() => {
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

    if (props.showMatches) {
      if (props.matches) {
        // show matching result
        const imageData = imageContext.getImageData(
          0,
          0,
          imageCanvas.width,
          imageCanvas.height
        );
        overlayMatchingResult(imageData.data, imageCanvas.width, props.matches);
        imageContext.putImageData(imageData, 0, 0);
      } else {
        console.error("Can't display matches. Matches are not set");
        return;
      }
    } else {
      console.log("Hide matches");
      // hide matches
      if (image) {
        imageContext.drawImage(image, 0, 0);
      }
    }
  }, [props.matches, props.showMatches, image]);

  return (
    <div>
      <div
        className="costMatrix"
        style={{
          height: props.height,
          width: props.width,
          maxHeight: "500px",
          maxWidth: "800px",
          overflow: "auto",
          margin: "10px",
          zIndex: 2,
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
