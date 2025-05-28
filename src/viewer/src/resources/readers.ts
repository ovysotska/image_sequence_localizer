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

enum ProtoMessageType {
  CostMatrix = "CostMatrix",
  MatchingResult = "MatchingResult",
  MatchingCosts = "MatchingCosts",
}

function readProtoFromBuffer(buffer: Uint8Array, protoMessageType: string) {
  return new Promise((resolve, reject) => {
    const protobuf = require("protobufjs");
    protobuf
      .load("localization_protos.proto")
      .then(function (root: any) {
        // Get the message type from the root object
        const message = root.lookupType(protoMessageType);
        const decodedMessage = message.decode(buffer);
        resolve(decodedMessage);
      })
      .catch((error: any) => {
        console.log("ERROR, proto couldn't be loaded", error);
        console.log("For type", protoMessageType);
        reject();
      });
  });
}

async function readProtoFromFile(file: File, messageType: ProtoMessageType) {
  try {
    let contentBuffer = await readFileAsync(file);
    if (contentBuffer == null) {
      throw new Error("Empty content");
    }
    let protoMessage = await readProtoFromBuffer(
      new Uint8Array(contentBuffer),
      "image_sequence_localizer." + messageType.toString()
    );
    console.log("Read proto message", protoMessage);
    return protoMessage;
  } catch (err) {
    console.log("File was not processed", err);
  }
}

type MatchingResultElement = {
  queryId: number;
  refId: number;
  real: boolean;
};

function readMatchingResultFromProto(
  matchingResultProto?: any
): MatchingResultElement[] | undefined {
  if (matchingResultProto == null) {
    return undefined;
  }
  return matchingResultProto.matches;
}

export {
  readFileAsync,
  readImageAsync,
  readProtoFromFile,
  readMatchingResultFromProto,
  ProtoMessageType,
};

export type { MatchingResultElement };
