import React, { useRef } from 'react';
import { motion, useScroll, useSpring, useTransform } from 'framer-motion';
import { Code, Download, Terminal, Monitor, Apple, GitBranch, MessageSquare } from 'lucide-react';

const RELEASE_URL = "https://github.com/abhyuday-fr/FlowPlusPlus/releases/download/v1.0.4";

function App() {
  const containerRef = useRef<HTMLDivElement>(null);
  const { scrollYProgress } = useScroll({
    target: containerRef,
    offset: ["start start", "end end"]
  });

  const scaleY = useSpring(scrollYProgress, {
    stiffness: 100,
    damping: 30,
    restDelta: 0.001
  });

  // This transform ensures the line stops exactly inside the End node
  const pathLength = useTransform(scaleY, [0, 0.94], [0, 1]);

  const nodeVariants = {
    off: { 
      boxShadow: "0 10px 30px rgba(0,0,0,0.5)",
      borderColor: "var(--node-border)",
      scale: 1 
    },
    glow: { 
      boxShadow: "0 0 25px var(--line-color), 0 0 50px rgba(35, 134, 54, 0.2)",
      borderColor: "var(--line-color)",
      scale: 1.02,
      transition: { duration: 0.5 }
    }
  };

  return (
    <div ref={containerRef} className="flow-container">
      <div className="canvas-bg" />

      {/* Dynamic SVG Flow Line */}
      <svg className="svg-container" style={{ width: '100%', height: '100%' }}>
        <motion.line
          x1="50%" y1="100"
          x2="50%" y2="92%" 
          stroke="var(--line-color)"
          strokeWidth="4"
          style={{ pathLength }}
        />
      </svg>

      {/* HERO NODE */}
      <motion.div 
        initial="off"
        whileInView="glow"
        viewport={{ amount: 0.8 }}
        variants={nodeVariants}
        className="node node-start"
        style={{ marginBottom: '250px' }}
      >
        <h1 className="hero-text">FlowPlusPlus</h1>
        <p style={{ fontSize: '1.2rem' }}>Visual Logic. Simplified.</p>
      </motion.div>

      {/* PROJECT NODE */}
      <motion.div 
        initial="off"
        whileInView="glow"
        viewport={{ amount: 0.8 }}
        variants={nodeVariants}
        className="node node-process"
        style={{ marginBottom: '250px' }}
      >
        <h2>The Project</h2>
        <p>Explore the source code, report bugs, or help us build the future of visual programming.</p>
        <div className="btn-group">
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus" className="btn btn-github">
            <Code size={18} /> Source
          </a>
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus/issues" className="btn btn-github">
            <MessageSquare size={18} /> Issues
          </a>
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus/pulls" className="btn btn-github">
            <GitBranch size={18} /> Contribute
          </a>
        </div>
      </motion.div>

      {/* DECISION NODE */}
      <motion.div 
        initial="off"
        whileInView="glow"
        viewport={{ amount: 0.8 }}
        variants={nodeVariants}
        className="node node-decision"
        style={{ marginBottom: '200px' }}
      >
        <h2 style={{ margin: 0 }}>Choose OS</h2>
      </motion.div>

      {/* DOWNLOADS */}
      <div style={{ display: 'flex', gap: '30px', flexWrap: 'wrap', justifyContent: 'center', marginBottom: '250px', width: '100%', maxWidth: '1200px', zIndex: 10 }}>
        <motion.div 
          initial="off"
          whileInView="glow"
          variants={nodeVariants}
          className="node node-process"
          style={{ flex: 1, minWidth: '320px', padding: '50px 30px', margin: 0 }}
        >
          <Terminal size={40} color="#d4a017" style={{ marginBottom: '20px' }} />
          <h3>Linux</h3>
          <p>Flatpak Bundle</p>
          <a href={`${RELEASE_URL}/FlowPlusPlus-linux.flatpak`} className="btn btn-linux">
            <Download size={18} /> Download
          </a>
        </motion.div>

        <motion.div 
          initial="off"
          whileInView="glow"
          variants={nodeVariants}
          className="node node-process"
          style={{ flex: 1, minWidth: '320px', padding: '50px 30px', margin: 0 }}
        >
          <Monitor size={40} color="#0078d4" style={{ marginBottom: '20px' }} />
          <h3>Windows</h3>
          <p>Portable Zip</p>
          <a href={`${RELEASE_URL}/FlowPlusPlus-windows-x64.zip`} className="btn btn-windows">
            <Download size={18} /> Download
          </a>
        </motion.div>

        <motion.div 
          initial="off"
          whileInView="glow"
          variants={nodeVariants}
          className="node node-process"
          style={{ flex: 1, minWidth: '320px', padding: '50px 30px', margin: 0 }}
        >
          <Apple size={40} color="#ffffff" style={{ marginBottom: '20px' }} />
          <h3>macOS</h3>
          <p>Apple DMG</p>
          <a href={`${RELEASE_URL}/FlowPlusPlus-macos.dmg`} className="btn btn-macos">
            <Download size={18} /> Download
          </a>
        </motion.div>
      </div>

      {/* STOP NODE */}
      <motion.div 
        initial="off"
        whileInView="glow"
        viewport={{ amount: 0.8 }}
        variants={nodeVariants}
        className="node node-start"
        style={{ marginBottom: 0, borderRadius: '100px', padding: '40px 100px' }}
      >
        <h2 style={{ fontSize: '2.5rem' }}>End</h2>
        <p>Happy Flowing!</p>
      </motion.div>

      <footer style={{ marginTop: '100px', paddingBottom: '40px', color: 'var(--text-dim)', textAlign: 'center' }}>
        <p>© 2026 FlowPlusPlus Open Source Project</p>
      </footer>
    </div>
  );
}

export default App;
