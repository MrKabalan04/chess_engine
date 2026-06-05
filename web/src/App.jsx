import { useState } from "react";
import Header from "./components/Header";
import ChessBoard from "./components/ChessBoard";
import Landing from "./components/Landing";
import Stars from "./components/Stars";      // ✅ import Stars
import "./App.css";

function App() {
  const [entered, setEntered] = useState(false);

  return (
    <div className="app">
      <Stars />   {/* ✅ Stars always present, behind everything */}
      {!entered ? (
        <Landing onEnter={() => setEntered(true)} />
      ) : (
        <>
          <Header />
          <div className="center">
            <ChessBoard />
          </div>
        </>
      )}
    </div>
  );
}

export default App;