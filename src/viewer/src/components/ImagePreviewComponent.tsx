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

  function updateGlobalQueryId(queryId?: number) {
    if (queryId == null) {
      return;
    }
    if (globalSelectedElement != null) {
      setGlobalSelectedElement({
        queryId: queryId,
        referenceId: globalSelectedElement?.referenceId,
      });
    }
  }

  function updateGlobalRefId(refId?: number) {
    if (refId == null) {
      return;
    }
    if (globalSelectedElement != null) {
      setGlobalSelectedElement({
        queryId: globalSelectedElement.queryId,
        referenceId: refId,
      });
    }
  }

  return (
    <div
      className="imageLoaders"
      style={{
        margin: "0 10px 0 10px",
        backgroundColor: "#EAF1FF",
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
          setSelectedImageId={updateGlobalQueryId}
        />
      </div>
      <div>
        <ImageCarousel
          imageFiles={props.referenceImageFiles}
          imageSource={"Reference"}
          showImageId={globalSelectedElement?.referenceId}
          setSelectedImageId={updateGlobalRefId}
        />
      </div>
    </div>
  );
}

export { ImagePreviewComponent };
