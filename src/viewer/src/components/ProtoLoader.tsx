import React, { useEffect, useState } from "react";

enum MessageType {
  CostMatrix = "CostMatrix",
  MatchingResult = "MatchingResult",
}

function readFileAsync(file: Blob) {
  return new Promise<ArrayBuffer | null>((resolve, reject) => {
    let reader = new FileReader();
    reader.onload = () => {
      resolve(reader.result as ArrayBuffer);
    };
    reader.onerror = reject;
    reader.readAsArrayBuffer(file);
  });
}

function readProto(buffer: Uint8Array, protoMessageType: string) {
  return new Promise((resolve, reject) => {
    const protobuf = require("protobufjs");
    protobuf
      .load("localization_protos.proto")
      .then(function (root: any) {
        // Get the message type from the root object
        const message = root.lookupType(protoMessageType);
        const decodedMessage = message.decode(buffer);
        console.log("Message", decodedMessage);
        resolve(decodedMessage);
      })
      .catch((error: any) => {
        console.log("ERROR, proto couldn't be loaded", error);
        reject();
      });
  });
}

type ProtoLoaderProps = {
  onLoad: (protoMessage: any) => void;
  messageType: MessageType;
};

function ProtoLoader(props: ProtoLoaderProps): React.ReactElement {
  const [protoMessage, setProtoMessage] = useState<any>();

  async function processFile(event: React.FormEvent<HTMLInputElement>) {
    try {
      const target = event.target as HTMLInputElement & { files: FileList };
      if (!target.files || target.files.length <= 0) {
        console.log("No file was uploaded");
        return;
      }

      let contentBuffer = await readFileAsync(target.files[0]);
      if (contentBuffer == null) {
        throw new Error("Empty content");
      }
      let protoMessage = await readProto(
        new Uint8Array(contentBuffer),
        "image_sequence_localizer." + props.messageType.toString()
      );
      console.log("Read proto message", protoMessage);
      setProtoMessage(protoMessage);
    } catch (err) {
      console.log("File was not processed", err);
    }
  }

  const { onLoad } = props;
  useEffect(() => {
    console.log("Proto state changed", protoMessage);
    onLoad(protoMessage);
  }, [protoMessage, onLoad]);

  return (
    <div>
      <label htmlFor="folder">
        Select <b>*.{props.messageType.toString()}.proto</b> file{" "}
      </label>
      <input type="file" id="fileInput" onChange={processFile}></input>
    </div>
  );
}

export { ProtoLoader, MessageType };
