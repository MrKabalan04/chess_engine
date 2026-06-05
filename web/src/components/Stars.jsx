import { useEffect, useRef } from "react";
import "./Stars.css";

function Stars() {
  const canvasRef = useRef(null);
  const animationRef = useRef(null);
  const mouseRef = useRef({ x: 0, y: 0 });

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext("2d");

    let width = window.innerWidth;
    let height = window.innerHeight;
    canvas.width = width;
    canvas.height = height;

    // Elegant starfield: fewer stars, gentle motion, varied sizes
    const initStars = (w, h) => {
      const count = 140; // calm, not crowded
      const stars = [];
      
      for (let i = 0; i < count; i++) {
        // Occasionally create a brighter, larger star
        const isBright = Math.random() < 0.1;
        stars.push({
          x: Math.random() * w,
          y: Math.random() * h,
          radius: isBright ? Math.random() * 2.5 + 2.5 : Math.random() * 1.8 + 0.8,
          speed: Math.random() * 0.25 + 0.08, // very slow drift
          depth: Math.random() * 0.6 + 0.2,
          alpha: isBright ? 0.8 : Math.random() * 0.4 + 0.3,
          twinkleSpeed: Math.random() * 0.008 + 0.002,
          twinklePhase: Math.random() * Math.PI * 2,
        });
      }
      return stars;
    };

    let stars = initStars(width, height);
    let time = 0;

    const drawStar = (ctx, x, y, radius, alpha, twinkleFactor) => {
      const finalAlpha = Math.min(0.85, alpha * twinkleFactor);
      const grad = ctx.createRadialGradient(x, y, radius * 0.15, x, y, radius * 1.2);
      grad.addColorStop(0, `rgba(255, 245, 210, ${finalAlpha})`);
      grad.addColorStop(0.6, `rgba(255, 200, 120, ${finalAlpha * 0.5})`);
      grad.addColorStop(1, `rgba(180, 120, 60, 0)`);
      ctx.fillStyle = grad;
      ctx.beginPath();
      ctx.arc(x, y, radius, 0, Math.PI * 2);
      ctx.fill();
    };

    const draw = () => {
      if (!ctx) return;
      ctx.clearRect(0, 0, width, height);

      const mouseX = mouseRef.current.x - width / 2;
      const mouseY = mouseRef.current.y - height / 2;

      for (let s of stars) {
        // Gentle downward drift
        s.y += s.speed;
        if (s.y > height + 20) {
          s.y = -20;
          s.x = Math.random() * width;
        }

        // Parallax – subtle
        const offsetX = mouseX * s.depth * 0.008;
        const offsetY = mouseY * s.depth * 0.008;
        const drawX = s.x + offsetX;
        const drawY = s.y + offsetY;

        // Soft twinkle
        const twinkle = 0.7 + 0.3 * Math.sin(time * s.twinkleSpeed + s.twinklePhase);
        
        drawStar(ctx, drawX, drawY, s.radius, s.alpha, twinkle);
      }

      time += 0.05; // slow twinkle evolution
      animationRef.current = requestAnimationFrame(draw);
    };

    draw();

    const handleMouseMove = (e) => {
      mouseRef.current.x = e.clientX;
      mouseRef.current.y = e.clientY;
    };

    const handleResize = () => {
      if (animationRef.current) cancelAnimationFrame(animationRef.current);
      width = window.innerWidth;
      height = window.innerHeight;
      canvas.width = width;
      canvas.height = height;
      stars = initStars(width, height);
      draw();
    };

    window.addEventListener("mousemove", handleMouseMove);
    window.addEventListener("resize", handleResize);

    return () => {
      window.removeEventListener("mousemove", handleMouseMove);
      window.removeEventListener("resize", handleResize);
      if (animationRef.current) cancelAnimationFrame(animationRef.current);
    };
  }, []);

  return <canvas ref={canvasRef} className="stars" />;
}

export default Stars;