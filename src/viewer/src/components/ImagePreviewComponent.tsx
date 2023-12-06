import React from "react";
import { ImageCarousel } from "./ImageCarousel";

import { useElementContext } from "../context/ElementContext";

type ImagePreviewComponentProps = {
  queryImageFiles?: File[];
  referenceImageFiles?: File[];
};

function ImagePreviewComponent(
  props: ImagePreviewComponentProps
): React.ReactElement {
  const { globalSelectedElement, setGlobalSelectedElement } =
    useElementContext();

  return (
    <div
      className="imageLoaders"
      style={{
        margin: "0 10px 0 10px",
        backgroundColor: "lavender",
        display: "flex",
        flexDirection: "row",
        flexWrap: "wrap",
        justifyContent: "center",
        columnGap: "20px",
      }}
    >
      <div>
        <ImageCarousel
          imageFiles={props.queryImageFiles}
          imageSource={"Query"}
          showImageId={globalSelectedElement?.queryId}
        />
      </div>
      <div>
        <ImageCarousel
          imageFiles={props.referenceImageFiles}
          imageSource={"Reference"}
          showImageId={globalSelectedElement?.refId}
        />
      </div>
    </div>
  );
}

export { ImagePreviewComponent };
