{
  "version": 1,
  "nodes": [
    { "id": 0, "type": "StartStop", "mode": "Start", "label": "Start", "x": 0, "y": -200 },
    { "id": 1, "type": "IO", "mode": "Output", "label": "\"Hello, World!\"", "x": 0, "y": -80 },
    { "id": 2, "type": "IO", "mode": "Output", "label": "\"Welcome to FlowPlusPlus!\"", "x": 0, "y": 40 },
    { "id": 3, "type": "StartStop", "mode": "Stop", "label": "Stop", "x": 0, "y": 160 }
  ],
  "connections": [
    { "from": 0, "to": 1, "isYes": false },
    { "from": 1, "to": 2, "isYes": false },
    { "from": 2, "to": 3, "isYes": false }
  ]
}
