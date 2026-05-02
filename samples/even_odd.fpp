{
  "version": 1,
  "nodes": [
    { "id": 0, "type": "StartStop", "mode": "Start", "label": "Start", "x": 0, "y": -320 },
    { "id": 1, "type": "IO", "mode": "Input", "label": "n", "x": 0, "y": -200 },
    { "id": 2, "type": "Process", "label": "remainder = n % 2", "x": 0, "y": -80 },
    { "id": 3, "type": "Decision", "label": "remainder == 0", "x": 0, "y": 40 },
    { "id": 4, "type": "IO", "mode": "Output", "label": "\"Even!\"", "x": -200, "y": 160 },
    { "id": 5, "type": "IO", "mode": "Output", "label": "\"Odd!\"", "x": 200, "y": 160 },
    { "id": 6, "type": "StartStop", "mode": "Stop", "label": "Stop", "x": 0, "y": 280 }
  ],
  "connections": [
    { "from": 0, "to": 1, "isYes": false },
    { "from": 1, "to": 2, "isYes": false },
    { "from": 2, "to": 3, "isYes": false },
    { "from": 3, "to": 4, "isYes": true },
    { "from": 3, "to": 5, "isYes": false },
    { "from": 4, "to": 6, "isYes": false },
    { "from": 5, "to": 6, "isYes": false }
  ]
}
