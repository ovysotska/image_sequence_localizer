import React, { createContext, useContext, ReactNode, useState } from "react";

type SelectedElement = {
  queryId: number;
  referenceId: number;
};

interface ElementContextProps {
  globalSelectedElement?: SelectedElement;
  setGlobalSelectedElement: React.Dispatch<
    React.SetStateAction<SelectedElement | undefined>
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
    useState<SelectedElement>();

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
    throw new Error("useElementContext must be used within an ElementProvider");
  }

  return context;
}

export { ElementProvider, useElementContext };
export type { SelectedElement };
