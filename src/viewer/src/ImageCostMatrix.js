import React from "react";
import { useState, useRef, useEffect } from "react";

const kZoomWindowPx = 50;

function ImageCostMatrix({ image }) {
  const canvasRef = useRef(null);
  const pixelZoomRef = useRef(null);
  const [imageSize, setImageSize] = useState(null);

  function onHoverOverImage(event) {
    let x, y;
    x = event.nativeEvent.layerX;
    y = event.nativeEvent.layerY;
    zoom(pixelZoomRef.current.getContext("2d"), x, y);
  }

  // Assumptions: Canvas size is adapted to image size.
  // Zoom image is always rescaled to 300px x 300px
  useEffect(() => {
    // Default for Firefox area:
    // canvasRef.current.width = 300;
    // canvasRef.current.height = 150;

    pixelZoomRef.current.width = 300;
    pixelZoomRef.current.height = 300;

    const canvas = canvasRef.current;

    const context = canvas.getContext("2d");
    if (image !== null) {
      console.log("Image size", image.width, image.height);
      setImageSize({ width: image.width, height: image.height });
      canvasRef.current.width = image.width;
      canvasRef.current.height = image.height;
      // Draw image of exact size.
      context.drawImage(image, 0, 0);
    } else {
      console.log("Empty image.", image);
    }

    const zoomContext = pixelZoomRef.current.getContext("2d");
    zoomContext.imageSmoothingEnabled = false;
  }, [image]);

  const zoom = (ctx, x, y) => {
    if (imageSize == null) {
      return;
    }
    const zoomBbox = {
      xStart: Math.min(
        Math.max(0, x - kZoomWindowPx / 2),
        imageSize.width - kZoomWindowPx
      ),
      yStart: Math.min(
        Math.max(0, y - kZoomWindowPx / 2),
        imageSize.height - kZoomWindowPx
      ),
      width: kZoomWindowPx,
      heigt: kZoomWindowPx,
    };
    ctx.drawImage(
      canvasRef.current,
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
        <canvas ref={canvasRef} onMouseMove={onHoverOverImage} />
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

export default ImageCostMatrix;
