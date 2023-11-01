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

export { type MatchingResultElement };
export { readMatchingResultFromProto };
