import { useRef, useState } from 'react';
import { motion, useScroll, useTransform, useMotionValueEvent, MotionValue } from 'framer-motion';
import { Code, Download, GitBranch, MessageSquare } from 'lucide-react';
import linuxLogo from './assets/linux.svg';
import macosLogo from './assets/macos.png';

const RELEASE_URL = "https://github.com/abhyuday-fr/FlowPlusPlus/releases/download/v1.0.6";

// C++ brand blues
const CPP_BLUE      = "#0054A6";
const CPP_MID       = "#0078D7";
const CPP_GLOW_DARK = "rgba(0,84,166,0.35)";

// Hover tooltip content
const TOOLTIPS: Record<string, { title: string; body: string }> = {
  start: {
    title: "What is FlowPlusPlus?",
    body: "FlowPlusPlus is a desktop application that lets you write programs by drawing flowcharts. Instead of staring at a blank text editor and trying to remember syntax, you place nodes on a canvas, connect them with arrows, and hit Run. The interpreter walks your graph, node by node, arrow by arrow, and executes your logic in real time. It is not a flowchart generator that produces pretty diagrams of code you already wrote. It is not a visual wrapper around a scripting language. It is a complete self contained interpreter: Lexer, Parser, AST, and Graph Walker, all built from scratch in C++17, all working together so that the diagram you draw IS the program that runs.",
  },
  project: {
    title: "Open Source",
    body: "Built entirely from scratch in C++17 and Qt 6, following the structure of Crafting Interpreters by Robert Nystrom. Every piece: Lexer, Parser, AST, Graph Walker, written by hand. MIT licensed and free forever.",
  },
  decision: {
    title: "Pick Your Platform",
    body: "FlowPlusPlus runs natively on Linux, Windows, and macOS. All three builds are generated automatically via GitHub Actions on every release.",
  },
  linux: {
    title: "Linux: Flatpak",
    body: "Distributed as a Flatpak bundle. Works on any modern Linux distro including Fedora, Ubuntu, Arch, and more. Install with one command and it just works.",
  },
  windows: {
    title: "Windows: Portable Zip",
    body: "No installer needed. Extract the zip and run FlowPlusPlus.exe directly. All Qt DLLs are bundled inside. Tested on Windows 10 and 11.",
  },
  macos: {
    title: "macOS: DMG",
    body: "Standard macOS disk image. Open, drag to Applications, done. Built for Apple Silicon and Intel Macs.",
  },
  end: {
    title: "You made it.",
    body: "The canvas is waiting. Go build something.",
  },
};

function Tooltip({ id, visible }: { id: string; visible: boolean }) {
  const t = TOOLTIPS[id];
  if (!t) return null;
  return (
    <motion.div
      className="tooltip"
      initial={{ opacity: 0, x: -12 }}
      animate={visible ? { opacity: 1, x: 0 } : { opacity: 0, x: -12 }}
      transition={{ duration: 0.22, ease: "easeOut" }}
      style={{ pointerEvents: 'none' }}
    >
      <div className="tooltip-title">{t.title}</div>
      <div className="tooltip-body">{t.body}</div>
    </motion.div>
  );
}

function FlowLineSegment({ progress, range }: { progress: MotionValue<number>; range: [number, number] }) {
  const scaleY = useTransform(progress, range, [0, 1]);
  return (
    <div className="line-segment">
      <motion.div className="flow-line-inner" style={{ scaleY }} />
    </div>
  );
}

function BranchingLines({ progress, range }: { progress: MotionValue<number>; range: [number, number] }) {
  const pathLength = useTransform(progress, range, [0, 1]);
  return (
    <div className="branch-container">
      <svg className="branch-svg" viewBox="0 0 1008 260" preserveAspectRatio="none">
        {/* Main stem */}
        <motion.path
          d="M 504 0 L 504 60"
          className="branch-path"
          style={{ pathLength }}
        />
        {/* Horizontal split */}
        <motion.path
          d="M 160 60 L 848 60"
          className="branch-path"
          style={{ pathLength }}
        />
        {/* Three branches down */}
        <motion.path d="M 160 60 L 160 260" className="branch-path" style={{ pathLength }} />
        <motion.path d="M 504 60 L 504 260" className="branch-path" style={{ pathLength }} />
        <motion.path d="M 848 60 L 848 260" className="branch-path" style={{ pathLength }} />
      </svg>
    </div>
  );
}

function ConvergingLines({ progress, range }: { progress: MotionValue<number>; range: [number, number] }) {
  const pathLength = useTransform(progress, range, [0, 1]);
  return (
    <div className="branch-container">
      <svg className="branch-svg" viewBox="0 0 1008 260" preserveAspectRatio="none">
        {/* Three branches up */}
        <motion.path d="M 160 0 L 160 200" className="branch-path" style={{ pathLength }} />
        <motion.path d="M 504 0 L 504 200" className="branch-path" style={{ pathLength }} />
        <motion.path d="M 848 0 L 848 200" className="branch-path" style={{ pathLength }} />
        {/* Horizontal join */}
        <motion.path
          d="M 160 200 L 848 200"
          className="branch-path"
          style={{ pathLength }}
        />
        {/* Main stem down */}
        <motion.path
          d="M 504 200 L 504 260"
          className="branch-path"
          style={{ pathLength }}
        />
      </svg>
    </div>
  );
}

const WindowsIcon = () => (
  <svg width="36" height="36" viewBox="0 0 24 24" fill="#0078d4">
    <path d="M0 3.449L9.75 2.1v9.451H0V3.449zM0 12.45h9.75v9.451L0 20.551V12.45zM10.71 1.95L24 0v11.55h-13.29V1.95zm0 10.5H24V24l-13.29-1.95V12.45z" />
  </svg>
);

export default function App() {
  const containerRef = useRef<HTMLDivElement>(null);
  const [hovered, setHovered]     = useState<string | null>(null);
  const [activeNode, setActiveNode] = useState(0);

  const { scrollYProgress } = useScroll({
    target: containerRef,
    offset: ["start start", "end end"],
  });

  // Glow thresholds — when the line visually reaches each node
  useMotionValueEvent(scrollYProgress, "change", (v) => {
    if      (v < 0.12) setActiveNode(0);
    else if (v < 0.35) setActiveNode(1);
    else if (v < 0.55) setActiveNode(2);
    else if (v < 0.78) setActiveNode(3);
    else               setActiveNode(4);
  });

  const cpp = (i: number) => ({
    animate: activeNode >= i ? "glow" : "off" as "glow" | "off",
  });

  const cppVar = {
    off: {
      boxShadow: "0 8px 32px rgba(0,0,0,0.6)",
      borderColor: "#1e2a3a",
      scale: 1,
      transition: { duration: 0.5 },
    },
    glow: {
      boxShadow: `0 0 28px ${CPP_BLUE}, 0 0 56px ${CPP_GLOW_DARK}`,
      borderColor: CPP_MID,
      scale: 1.02,
      transition: { duration: 0.4 },
    },
  };

  const greenVar = {
    off: {
      boxShadow: "0 8px 32px rgba(0,0,0,0.6)",
      borderColor: "#238636",
      scale: 1,
      transition: { duration: 0.5 },
    },
    glow: {
      boxShadow: "0 0 28px #238636, 0 0 56px rgba(35,134,54,0.3)",
      borderColor: "#3fb950",
      scale: 1.02,
      transition: { duration: 0.4 },
    },
  };

  const hover = (id: string) => ({
    onMouseEnter: () => setHovered(id),
    onMouseLeave: () => setHovered(null),
  });

  return (
    <div ref={containerRef} className="flow-container">
      <div className="canvas-bg" />

      {/* START */}
      <motion.div
        variants={greenVar} {...cpp(0)}
        className="node node-start"
        {...hover('start')}
      >
        <div className="logo-badge">F++</div>
        <h1 className="hero-text">FlowPlusPlus</h1>
        <p className="hero-sub">Visual Logic. Executed.</p>
        <p className="hero-desc">
          A flowchart that actually runs, built with C++17 and Qt 6.
        </p>
        <Tooltip id="start" visible={hovered === 'start'} />
      </motion.div>

      <FlowLineSegment progress={scrollYProgress} range={[0.08, 0.22]} />

      {/* PROJECT */}
      <motion.div
        variants={cppVar} {...cpp(1)}
        className="node node-process"
        {...hover('project')}
      >
        <div className="node-tag">PROCESS</div>
        <h2>The Project</h2>
        <p>
          Open source. Built from scratch. Explore the source, report bugs,
          or help shape the future of visual programming.
        </p>
        <div className="btn-group">
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus" className="btn btn-github">
            <Code size={15} /> Source
          </a>
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus/issues" className="btn btn-github">
            <MessageSquare size={15} /> Issues
          </a>
          <a href="https://github.com/abhyuday-fr/FlowPlusPlus/pulls" className="btn btn-github">
            <GitBranch size={15} /> Contribute
          </a>
        </div>
        <Tooltip id="project" visible={hovered === 'project'} />
      </motion.div>

      <FlowLineSegment progress={scrollYProgress} range={[0.32, 0.45]} />

      {/* DECISION */}
      <motion.div
        variants={cppVar} {...cpp(2)}
        className="node node-decision"
        {...hover('decision')}
      >
        <h2>Choose<br />Your OS</h2>
        <Tooltip id="decision" visible={hovered === 'decision'} />
      </motion.div>

      <BranchingLines progress={scrollYProgress} range={[0.52, 0.65]} />

      {/* DOWNLOADS */}
      <div className="download-row">
        {[
          { id: 'linux',   icon: <img src={linuxLogo} alt="Linux" style={{ height: '44px' }} />, title: "Linux",   sub: "Flatpak Bundle", cls: "btn-linux",   href: `${RELEASE_URL}/FlowPlusPlus-linux.flatpak` },
          { id: 'windows', icon: <WindowsIcon />, title: "Windows", sub: "Portable Zip",   cls: "btn-windows", href: `${RELEASE_URL}/FlowPlusPlus-windows-x64.zip` },
          { id: 'macos',   icon: <img src={macosLogo} alt="macOS" style={{ height: '40px' }} />, title: "macOS",   sub: "Apple DMG",      cls: "btn-macos",   href: `${RELEASE_URL}/FlowPlusPlus-macos.dmg` },
        ].map((d) => (
          <motion.div
            key={d.id}
            variants={cppVar} {...cpp(3)}
            className="node node-download"
            {...hover(d.id)}
          >
            <div className="node-tag">I/O</div>
            {d.icon}
            <h3>{d.title}</h3>
            <p>{d.sub}</p>
            <a href={d.href} className={`btn ${d.cls}`}>
              <Download size={14} /> Download
            </a>
            <Tooltip id={d.id} visible={hovered === d.id} />
          </motion.div>
        ))}
      </div>

      <ConvergingLines progress={scrollYProgress} range={[0.75, 0.88]} />

      {/* END — line stops here, no output port */}
      <motion.div
        variants={greenVar} {...cpp(4)}
        className="node node-start node-end"
        {...hover('end')}
      >
        <h2 className="end-text">End</h2>
        <p>Happy Flowing</p>
        <Tooltip id="end" visible={hovered === 'end'} />
      </motion.div>

      <footer>
        <p>2026 FlowPlusPlus: Open Source, MIT License</p>
        <p className="footer-sub">Built with C++17, Qt 6, CMake, Crafting Interpreters</p>
      </footer>
    </div>
  );
}
