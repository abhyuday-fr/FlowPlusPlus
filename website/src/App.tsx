import { useRef, useState } from 'react';
import { motion, useScroll, useSpring, useTransform, useMotionValueEvent } from 'framer-motion';
import { Code, Download, Terminal, Monitor, Apple, GitBranch, MessageSquare } from 'lucide-react';

const RELEASE_URL = "https://github.com/abhyuday-fr/FlowPlusPlus/releases/download/v1.0.4";

// C++ brand blue palette
const CPP_BLUE      = "#0054A6";   // deep C++ blue
const CPP_MID       = "#0078D7";   // mid blue
const CPP_LIGHT     = "#00A3FF";   // highlight
const CPP_GLOW_DARK = "rgba(0,84,166,0.35)";

// Node refs for position tracking
function useNodeGlow(scrollYProgress: ReturnType<typeof useScroll>["scrollYProgress"]) {
  const [activeNode, setActiveNode] = useState(-1);

  useMotionValueEvent(scrollYProgress, "change", (v) => {
    // Thresholds: when the line "reaches" each node
    if (v < 0.12)       setActiveNode(0);   // Start
    else if (v < 0.35)  setActiveNode(1);   // Project
    else if (v < 0.55)  setActiveNode(2);   // Decision
    else if (v < 0.78)  setActiveNode(3);   // Downloads
    else                setActiveNode(4);   // End
  });

  return activeNode;
}

function App() {
  const containerRef = useRef<HTMLDivElement>(null);

  const { scrollYProgress } = useScroll({
    target: containerRef,
    offset: ["start start", "end end"]
  });

  const rawSpring = useSpring(scrollYProgress, {
    stiffness: 80,
    damping: 25,
    restDelta: 0.001
  });

  // Line stops just before the End node (~88% scroll)
  const pathLength = useTransform(rawSpring, [0, 0.88], [0, 1]);

  const activeNode = useNodeGlow(scrollYProgress);

  const nodeGlow = (index: number) => ({
    animate: activeNode >= index ? "glow" : "off",
  });

  const variants = {
    off: {
      boxShadow: `0 8px 32px rgba(0,0,0,0.6)`,
      borderColor: "#1e2a3a",
      scale: 1,
      transition: { duration: 0.6 }
    },
    glow: {
      boxShadow: `0 0 30px ${CPP_BLUE}, 0 0 60px ${CPP_GLOW_DARK}, 0 8px 32px rgba(0,0,0,0.6)`,
      borderColor: CPP_MID,
      scale: 1.02,
      transition: { duration: 0.5 }
    }
  };

  const endVariants = {
    off: {
      boxShadow: `0 8px 32px rgba(0,0,0,0.6)`,
      borderColor: "#238636",
      scale: 1,
      transition: { duration: 0.6 }
    },
    glow: {
      boxShadow: `0 0 30px #238636, 0 0 60px rgba(35,134,54,0.3), 0 8px 32px rgba(0,0,0,0.6)`,
      borderColor: "#3fb950",
      scale: 1.02,
      transition: { duration: 0.5 }
    }
  };

  return (
    <div ref={containerRef} className="flow-container">
      <div className="canvas-bg" />

      {/* SVG flow line — stops at 88% page height (just above End node) */}
      <svg className="svg-container">
        <defs>
          <linearGradient id="lineGrad" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%"   stopColor={CPP_LIGHT} />
            <stop offset="60%"  stopColor={CPP_MID} />
            <stop offset="100%" stopColor={CPP_BLUE} />
          </linearGradient>
        </defs>
        <motion.line
          x1="50%" y1="120"
          x2="50%" y2="88%"
          stroke="url(#lineGrad)"
          strokeWidth="3"
          strokeLinecap="round"
          style={{ pathLength }}
          filter="url(#glow)"
        />
        <defs>
          <filter id="glow">
            <feGaussianBlur stdDeviation="3" result="blur" />
            <feMerge>
              <feMergeNode in="blur" />
              <feMergeNode in="SourceGraphic" />
            </feMerge>
          </filter>
        </defs>
      </svg>

      {/* ── START NODE ─────────────────────────────────── */}
      <motion.div
        variants={endVariants}
        {...nodeGlow(0)}
        className="node node-start"
        style={{ marginBottom: '260px' }}
      >
        <div className="logo-badge">F++</div>
        <h1 className="hero-text">FlowPlusPlus</h1>
        <p className="hero-sub">Visual Logic. Executed.</p>
        <p className="hero-desc">
          A flowchart that actually runs — built with C++17 and Qt 6.
        </p>
      </motion.div>

      {/* ── PROJECT NODE ───────────────────────────────── */}
      <motion.div
        variants={variants}
        {...nodeGlow(1)}
        className="node node-process"
        style={{ marginBottom: '260px' }}
      >
        <div className="node-tag">PROCESS</div>
        <h2>The Project</h2>
        <p>
          Open source. Built from scratch. Explore the source, report bugs,
          or help shape the future of visual programming.
        </p>
        <div className="btn-group">
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus" className="btn btn-github">
            <Code size={16} /> Source
          </a>
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus/issues" className="btn btn-github">
            <MessageSquare size={16} /> Issues
          </a>
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus/pulls" className="btn btn-github">
            <GitBranch size={16} /> Contribute
          </a>
        </div>
      </motion.div>

      {/* ── DECISION NODE ──────────────────────────────── */}
      <motion.div
        variants={variants}
        {...nodeGlow(2)}
        className="node node-decision"
        style={{ marginBottom: '260px' }}
      >
        <h2>Choose<br />Your OS</h2>
      </motion.div>

      {/* ── DOWNLOAD NODES ─────────────────────────────── */}
      <div className="download-row" style={{ marginBottom: '260px' }}>
        {[
          {
            icon: <Terminal size={36} color="#d4a017" />,
            title: "Linux",
            sub: "Flatpak Bundle",
            cls: "btn-linux",
            href: `${RELEASE_URL}/FlowPlusPlus-linux.flatpak`,
            index: 3,
          },
          {
            icon: <Monitor size={36} color="#0078d4" />,
            title: "Windows",
            sub: "Portable Zip",
            cls: "btn-windows",
            href: `${RELEASE_URL}/FlowPlusPlus-windows-x64.zip`,
            index: 3,
          },
          {
            icon: <Apple size={36} color="#ffffff" />,
            title: "macOS",
            sub: "Apple DMG",
            cls: "btn-macos",
            href: `${RELEASE_URL}/FlowPlusPlus-macos.dmg`,
            index: 3,
          },
        ].map((d) => (
          <motion.div
            key={d.title}
            variants={variants}
            {...nodeGlow(d.index)}
            className="node node-download"
          >
            <div className="node-tag">I/O</div>
            {d.icon}
            <h3>{d.title}</h3>
            <p>{d.sub}</p>
            <a href={d.href} className={`btn ${d.cls}`}>
              <Download size={15} /> Download
            </a>
          </motion.div>
        ))}
      </div>

      {/* ── END NODE ───────────────────────────────────── */}
      <motion.div
        variants={endVariants}
        {...nodeGlow(4)}
        className="node node-start node-end"
      >
        <h2 className="end-text">End</h2>
        <p>Happy Flowing 🌊</p>
      </motion.div>

      <footer>
        <p>© 2026 FlowPlusPlus — Open Source, MIT License</p>
        <p className="footer-sub">
          Built with C++17 · Qt 6 · CMake · Crafting Interpreters
        </p>
      </footer>
    </div>
  );
}

export default App;
