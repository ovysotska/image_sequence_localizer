import { useState, useEffect } from "react";

import "./ImageCarousel.css";

import ArrowForwardIosRoundedIcon from "@mui/icons-material/ArrowForwardIosRounded";
import ArrowBackIosRoundedIcon from "@mui/icons-material/ArrowBackIosRounded";

function readImageAsync(file: Blob) {
  return new Promise<string>((resolve, reject) => {
    let reader = new FileReader();
    reader.onload = () => {
      resolve(reader.result as string);
    };
    reader.onerror = reject;
    reader.readAsDataURL(file);
  });
}

type ImageData = {
  id: number;
  fileName: string;
  base64Encoding?: string;
};

type ImageCarouselProps = {
  imageFiles?: File[];
  imageSource: string;
  showImageId?: number;
  setSelectedImageId?: (id?: number) => void;
};

function ImageCarousel(props: ImageCarouselProps) {
  const [images, setImages] = useState<ImageData[]>();
  const [currentImageId, setCurrentImageId] = useState<number>(0);

  const { imageFiles, showImageId } = props;

  function loadImages(files?: File[]) {
    if (files == null) {
      return;
    }
    if (files.length === 0) {
      setImages(undefined);
    }

    const sortedFiles = Array.from(files).sort((a, b) => {
      return a.name.localeCompare(b.name);
    });

    const imageList = new Array<ImageData>(sortedFiles.length);
    sortedFiles.forEach((file, index) => {
      if (file) {
        readImageAsync(file)
          .then((imageAsBase64) => {
            imageList[index] = {
              id: index,
              fileName: file.name,
              base64Encoding: imageAsBase64,
            };
            setImages(imageList);
          })
          .catch((error) => {
            console.warn("Image was not loaded", error);
          });
      }
    });
  }

  useEffect(() => {
    // Loads from props
    loadImages(imageFiles);
  }, [imageFiles]);

  useEffect(() => {
    if (images == null || showImageId == null) {
      return;
    }
    if (showImageId < 0 || showImageId >= images.length) {
      return;
    }
    setCurrentImageId(showImageId);
  }, [showImageId, images]);

  function onChange(event: React.ChangeEvent<HTMLInputElement>) {
    // Loads from input
    event.preventDefault();
    let files = event.target.files;
    if (files == null) {
      return;
    }
    loadImages(Array.from(files));
  }

  function handleNextClick() {
    if (images == null || currentImageId == null) {
      return;
    }
    const nextImageId = Math.min(currentImageId + 1, images.length - 1);
    setCurrentImageId(nextImageId);
    if (props.setSelectedImageId != null) {
      props.setSelectedImageId(nextImageId);
    }
  }

  function handlePrevClick() {
    if (images == null || currentImageId == null) {
      return;
    }
    const prevImageId = Math.max(currentImageId - 1, 0);
    setCurrentImageId(prevImageId);
    if (props.setSelectedImageId != null) {
      props.setSelectedImageId(prevImageId);
    }
  }

  return (
    <div
      style={{
        textAlign: "center",
      }}
    >
      <h2> {props.imageSource} images</h2>

      {!images && (
        <div>
          <label htmlFor="folder">Select {props.imageSource} images </label>
          <input type="file" id="folder" multiple onChange={onChange} />
        </div>
      )}
      {images && currentImageId !== undefined &&
        images.length > currentImageId &&
        images[currentImageId] !== undefined  && 
        (
          <div className="imageCarousel">
            <img
              src={images[currentImageId].base64Encoding}
              alt="preview"
              className="slide"
            />
            <p className="info">
              id: {images[currentImageId].id}; filename:{" "}
              {images[currentImageId].fileName}
            </p>

            <ArrowForwardIosRoundedIcon
              className="arrow arrow-right"
              onClick={handleNextClick}
            ></ArrowForwardIosRoundedIcon>
            <ArrowBackIosRoundedIcon
              className="arrow arrow-left"
              onClick={handlePrevClick}
            ></ArrowBackIosRoundedIcon>
          </div>
        )} 
      <div></div>
    </div>
  );
}

export { ImageCarousel };
export type { ImageCarouselProps };
