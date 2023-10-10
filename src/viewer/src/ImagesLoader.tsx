import { useState, useEffect } from "react";

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

type ImageLoaderProps = {
  imageType: string;
  showImageId?: number;
};

function ImagesLoader(props: ImageLoaderProps) {
  const [images, setImages] = useState<ImageData[]>();
  const [currentImageId, setCurrentImageId] = useState<number>(0);

  useEffect(() => {
    console.log("Got signal to change showImageId");
    if (images == null || props.showImageId == null) {
      return;
    }
    if (props.showImageId < 0 || props.showImageId >= images.length) {
      return;
    }
    setCurrentImageId(props.showImageId);
  }, [props.showImageId, images]);

  function onChange(event: React.ChangeEvent<HTMLInputElement>) {
    event.preventDefault();
    let files = event.target.files;
    if (files == null) {
      return;
    }

    const sortedFiles = Array.from(files).sort((a, b) => {
      return a.name.localeCompare(b.name);
    });

    const imageList: ImageData[] = [];
    sortedFiles.forEach((file, index) => {
      if (file) {
        readImageAsync(file)
          .then((imageAsBase64) => {
            imageList.push({
              id: index,
              fileName: file.name,
              base64Encoding: imageAsBase64,
            });
            setImages(imageList);
          })
          .catch((error) => {
            console.log("Image was not loaded");
          });
      }
    });
  }

  function handleNextClick() {
    if (images == null) {
      return;
    }
    setCurrentImageId(Math.min(currentImageId + 1, images.length - 1));
  }

  function handlePrevClick() {
    if (images == null) {
      return;
    }
    setCurrentImageId(Math.max(currentImageId - 1, 0));
  }

  return (
    <div
      style={{
        textAlign: "center",
      }}
    >
      <h2>{props.imageType} images</h2>

      <div>
        <label htmlFor="folder">Select {props.imageType} images </label>
        <input type="file" id="folder" multiple onChange={onChange} />
      </div>
      {images && images.length > currentImageId && (
        <div>
          <img src={images[currentImageId].base64Encoding} alt="preview" />
          <p>
            id: {images[currentImageId].id}; filename:{" "}
            {images[currentImageId].fileName}
          </p>
        </div>
      )}
      <div>
        <button onClick={handlePrevClick}>prev</button>
        <button onClick={handleNextClick}>next</button>
      </div>
    </div>
  );
}

export { ImagesLoader };
export type { ImageLoaderProps };
