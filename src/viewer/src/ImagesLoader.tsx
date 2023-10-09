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

  async function loadImage(file: Blob) {
    if (file) {
      let imageAsBase64 = await readImageAsync(file);
      return imageAsBase64;
    }
  }

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

  // useEffect(() => {
  //   console.log("Changed state Images", images);
  //   console.log("state Images length", images?.length);
  // }, [images]);

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

  function handleButtonClick() {
    if (images == null) {
      return;
    }
    setCurrentImageId(Math.min(currentImageId + 1, images.length - 1));
  }

  return (
    <div>
      <label htmlFor="folder">Select {props.imageType} images</label>
      <input type="file" id="folder" multiple onChange={onChange} />
      <ul id="folder"></ul>
      {images && images.length > currentImageId && (
        <img src={images[currentImageId].base64Encoding} alt="preview" />
      )}
      <button onClick={handleButtonClick}>next</button>
    </div>
  );
}

export { ImagesLoader };
export type { ImageLoaderProps };
