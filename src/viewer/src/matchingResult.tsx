type MatchingResultElement = {
  queryId: number;
  refId: number;
  real: boolean;
};

function readMatchingResultFromProto(
  matchingResultProto?: any
): MatchingResultElement[] {
  if (matchingResultProto == null) {
    return [];
  }
  return matchingResultProto.matches;
}

export { type MatchingResultElement };
export { readMatchingResultFromProto };
