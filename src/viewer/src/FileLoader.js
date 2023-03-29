import { useEffect, useState } from "react";

function FileLoader({ onLoad }) {
  const [image, setImage] = useState(null);

  async function processFile() {
    try {
      const file = document.getElementById("fileInput").files[0];
      const imageBitmap = await createImageBitmap(file);
      setImage(imageBitmap);
    } catch (err) {
      console.log("File was not processed", err);
    }
  }

  useEffect(() => {
    console.log("File Loader got a new image", image);
    onLoad(image);
  }, [image]);

  return (
    <div style={{ width: "80%", padding: "10px", margin: "auto" }}>
      <label htmlFor="imageFile">Select cost matrix image file: </label>
      <input
        type="file"
        id="fileInput"
        name="imageFile"
        onChange={processFile}
      ></input>
    </div>
  );
}

export default FileLoader;
