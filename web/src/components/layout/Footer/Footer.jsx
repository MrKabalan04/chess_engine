import "./Footer.css";

function Footer() {
  return (
    <footer className="footer">
      <div className="footer-inner">
        <div className="line">
          Luna Engine • C++ / React / WebAssembly
        </div>

        <div className="line small">
          Built by Mr Kabalan • <a href="https://github.com/MrKabalan04" target="_blank" rel="noopener noreferrer">GitHub</a> • <a href="https://www.linkedin.com/in/mohamad-kabalan-b0378b301" target="_blank" rel="noopener noreferrer">LinkedIn</a>
        </div>

        <div className="line tiny">
          © 2026 Mr Kabalan
        </div>
      </div>
    </footer>
  );
}

export default Footer;