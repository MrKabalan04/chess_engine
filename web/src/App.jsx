import { useState } from "react";
import Header from "././components/layout/Header/Header";
import ChessBoard from "./components/ChessBoard";
import Landing from "./components/Landing";
import Stars from "./components/Stars";     
import Footer from "././components/layout/Footer/Footer";
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
          <Footer />
        </>
      )}
    </div>
  );
}

export default App;