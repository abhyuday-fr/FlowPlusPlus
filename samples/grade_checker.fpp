{
  "version": 1,
  "nodes": [
    { "id": 0, "type": "StartStop", "mode": "Start", "label": "Start", "x": 0, "y": -440 },
    { "id": 1, "type": "IO", "mode": "Input", "label": "score", "x": 0, "y": -320 },
    { "id": 2, "type": "Decision", "label": "score >= 90", "x": 0, "y": -200 },
    { "id": 3, "type": "IO", "mode": "Output", "label": "\"Grade: A\"", "x": -300, "y": -80 },
    { "id": 4, "type": "Decision", "label": "score >= 80", "x": 100, "y": -80 },
    { "id": 5, "type": "IO", "mode": "Output", "label": "\"Grade: B\"", "x": -100, "y": 40 },
    { "id": 6, "type": "Decision", "label": "score >= 70", "x": 200, "y": 40 },
    { "id": 7, "type": "IO", "mode": "Output", "label": "\"Grade: C\"", "x": 0, "y": 160 },
    { "id": 8, "type": "IO", "mode": "Output", "label": "\"Grade: F\"", "x": 300, "y": 160 },
    { "id": 9, "type": "StartStop", "mode": "Stop", "label": "Stop", "x": 0, "y": 280 }
  ],
  "connections": [
    { "from": 0, "to": 1, "isYes": false },
    { "from": 1, "to": 2, "isYes": false },
    { "from": 2, "to": 3, "isYes": true  },
    { "from": 2, "to": 4, "isYes": false },
    { "from": 3, "to": 9, "isYes": false },
    { "from": 4, "to": 5, "isYes": true  },
    { "from": 4, "to": 6, "isYes": false },
    { "from": 5, "to": 9, "isYes": false },
    { "from": 6, "to": 7, "isYes": true  },
    { "from": 6, "to": 8, "isYes": false },
    { "from": 7, "to": 9, "isYes": false },
    { "from": 8, "to": 9, "isYes": false }
  ]
}
