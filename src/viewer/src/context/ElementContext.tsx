import React, { createContext, useContext, ReactNode, useState } from "react";
import { CostMatrixElement } from "../resources/costMatrix";

interface ElementContextProps {
  globalSelectedElement?: CostMatrixElement;
  setGlobalSelectedElement: React.Dispatch<
    React.SetStateAction<CostMatrixElement | undefined>
  >;
}

const ElementContext = createContext<ElementContextProps | undefined>(
  undefined
);

interface ElementProviderProps {
  children: ReactNode;
}

function ElementProvider(props: ElementProviderProps) {
  const [globalSelectedElement, setGlobalSelectedElement] =
    useState<CostMatrixElement>();

  return (
    <ElementContext.Provider
      value={{ globalSelectedElement, setGlobalSelectedElement }}
    >
      {props.children}
    </ElementContext.Provider>
  );
}

function useElementContext(): ElementContextProps {
  const context = useContext(ElementContext);

  if (!context) {
    throw new Error("useAppContext must be used within an AppProvider");
  }

  return context;
}

export { ElementProvider, useElementContext };
