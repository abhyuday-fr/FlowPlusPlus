{
  "version": 1,
  "nodes": [
    { "id": 0, "type": "StartStop", "mode": "Start", "label": "Start", "x": 0, "y": -320 },
    { "id": 1, "type": "Process", "label": "n = 10", "x": 0, "y": -200 },
    { "id": 2, "type": "Decision", "label": "n > 0", "x": 0, "y": -80 },
    { "id": 3, "type": "IO", "mode": "Output", "label": "n", "x": -200, "y": 40 },
    { "id": 4, "type": "Process", "label": "n = n - 1", "x": -200, "y": 160 },
    { "id": 5, "type": "IO", "mode": "Output", "label": "\"Blastoff!\"", "x": 200, "y": 40 },
    { "id": 6, "type": "StartStop", "mode": "Stop", "label": "Stop", "x": 200, "y": 160 }
  ],
  "connections": [
    { "from": 0, "to": 1, "isYes": false },
    { "from": 1, "to": 2, "isYes": false },
    { "from": 2, "to": 3, "isYes": true },
    { "from": 2, "to": 5, "isYes": false },
    { "from": 3, "to": 4, "isYes": false },
    { "from": 4, "to": 2, "isYes": false },
    { "from": 5, "to": 6, "isYes": false }
  ]
}
