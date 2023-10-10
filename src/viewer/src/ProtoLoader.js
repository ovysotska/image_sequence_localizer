import { useEffect, useState } from "react";
import React from "react";

function readFileAsync(file) {
  return new Promise((resolve, reject) => {
    let reader = new FileReader();
    reader.onload = () => {
      resolve(reader.result);
    };
    reader.onerror = reject;
    reader.readAsArrayBuffer(file);
  });
}

function readProto(buffer) {
  return new Promise((resolve, reject) => {
    const protobuf = require("protobufjs");
    protobuf
      .load("localization_protos.proto")
      .then(function (root) {
        // Get the message type from the root object
        const costMatrixMessage = root.lookupType(
          "image_sequence_localizer.CostMatrix"
        );
        const decodedMessage = costMatrixMessage.decode(buffer);
        console.log("Message", decodedMessage);
        resolve(decodedMessage);
      })
      .catch((error) => {
        console.log("ERROR, proto couldn't be loaded", error);
        reject();
      });
  });
}

function ProtoLoader({ onLoad }) {
  const [costMatrixProto, setCostMatrixProto] = useState();

  async function processFile() {
    try {
      let file = document.getElementById("fileInput").files[0];
      let contentBuffer = await readFileAsync(file);
      console.log("Hello content", contentBuffer);
      let protoMessage = await readProto(new Uint8Array(contentBuffer));
      console.log("Proto message", protoMessage);
      setCostMatrixProto(protoMessage);
    } catch (err) {
      console.log("File was not processed", err);
    }
  }

  useEffect(() => {
    console.log("Cost Matrix changes", costMatrixProto);
    onLoad(costMatrixProto);
  }, [costMatrixProto, onLoad]);

  return (
    <div>
      <label htmlFor="folder">Select *.CostMatrix.proto file </label>
      <input type="file" id="fileInput" onChange={processFile}></input>
    </div>
  );
}

export default ProtoLoader;
