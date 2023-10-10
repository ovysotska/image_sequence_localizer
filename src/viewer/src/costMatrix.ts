type CostMatrixElement = {
  queryId: number;
  refId: number;
  value: number;
};

class CostMatrix {
  rows: number;
  cols: number;
  valuesArray: CostMatrixElement[];

  constructor(costMatrixProto?: any) {
    if (costMatrixProto == null) {
      this.rows = 0;
      this.cols = 0;
      this.valuesArray = [];
      return;
    }
    const costMatrixValues: CostMatrixElement[] = [];
    let valueIter = 0;
    for (let r = 0; r < costMatrixProto.rows; r += 1) {
      for (let c = 0; c < costMatrixProto.cols; c += 1) {
        costMatrixValues.push({
          queryId: r,
          refId: c,
          value: costMatrixProto.values[valueIter],
        });
        valueIter += 1;
      }
    }
    this.rows = costMatrixProto.rows;
    this.cols = costMatrixProto.cols;
    console.log(`Rows ${this.rows} cols ${this.cols}`);
    this.valuesArray = costMatrixValues;
  }

  async createImage() {
    const raw = [];
    for (let i = 0; i < this.valuesArray.length; i++) {
      const value = Math.floor(this.valuesArray[i].value * 255);
      raw.push(...[value, value, value, 255]);
    }
    const rawUint8 = new Uint8ClampedArray(raw);
    const imageData = new ImageData(rawUint8, this.cols, this.rows);
    const bitmap = await createImageBitmap(imageData);
    return bitmap;
  }

  indexToMatrixCoords(index: number) {
    const row = Math.floor(index / this.cols);
    const col = index - row * this.cols;
    return { row: row, col: col };
  }
  matrixCoordsToIndex(row: number, col: number): number {
    return row * this.cols + col;
  }

  getSubMatrix(topLeftCol: number, topLeftRow: number, windowSize: number) {
    if (topLeftCol == null || topLeftRow == null || windowSize == null) {
      console.warn("Boundaries for submatrix are not valid");
      return undefined;
    }
    // Making sure we have integer here
    topLeftCol = Math.floor(topLeftCol);
    topLeftRow = Math.floor(topLeftRow);
    // Can probably be speed up by using CopyWithin and giving the range.
    const subMatrixValues = [];
    for (let r = topLeftRow; r < topLeftRow + windowSize; r += 1) {
      for (let c = topLeftCol; c < topLeftCol + windowSize; c += 1) {
        if (r < 0 || r >= this.rows || c < 0 || c >= this.cols) {
          continue;
        }
        const index = this.matrixCoordsToIndex(r, c);
        subMatrixValues.push(this.valuesArray[index]);
      }
    }
    const subMatrix = new CostMatrix();
    subMatrix.rows = windowSize;
    subMatrix.cols = windowSize;
    subMatrix.valuesArray = subMatrixValues;
    return subMatrix;
  }
}

export { CostMatrix };
export type { CostMatrixElement };
