// Landing.js
import "./Landing.css";
import { useEffect, useState } from "react";

function Landing({ onEnter }) {
  const [visible, setVisible] = useState(false);

  useEffect(() => {
    // Gentle fade-in on mount
    setTimeout(() => setVisible(true), 50);
  }, []);

  return (
    <div className="landing" onClick={onEnter}>
      <div className={`title-container ${visible ? "visible" : ""}`}>
        <h1 className="luna-handwritten">Luna</h1>
        <div className="underline-glow"></div>
      </div>
      <div className={`sub ${visible ? "visible" : ""}`}>
        a quiet chess engine under the moon
      </div>
      <div className={`hint ${visible ? "visible" : ""}`}>
        click anywhere to begin
      </div>
    </div>
  );
}

export default Landing;