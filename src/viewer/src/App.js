// Copyright (c) 2023 O. Vysotska

import "./App.css";
import ImageCostMatrix from "./ImageCostMatrix";
import FileLoader from "./FileLoader";
import { useState } from "react";

function App() {
  const [image, setImage] = useState(null);

  return (
    <div>
      <h1 style={{ textAlign: "center" }}>Cost Matrix Viewer</h1>
      <FileLoader onLoad={setImage} />
      <div style={{ width: "80%", height: "100vh", margin: "auto" }}>
        {image && <ImageCostMatrix image={image} style={{ padding: "30px" }} />}
      </div>
    </div>
  );
}

export default App;
