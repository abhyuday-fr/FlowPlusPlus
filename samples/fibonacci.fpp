{
  "version": 1,
  "nodes": [
    { "id": 0, "type": "StartStop", "mode": "Start", "label": "Start", "x": 0, "y": -440 },
    { "id": 1, "type": "IO", "mode": "Input", "label": "n", "x": 0, "y": -320 },
    { "id": 2, "type": "Process", "label": "a = 0", "x": 0, "y": -200 },
    { "id": 3, "type": "Process", "label": "b = 1", "x": 0, "y": -80 },
    { "id": 4, "type": "Process", "label": "i = 0", "x": 0, "y": 40 },
    { "id": 5, "type": "Decision", "label": "i < n", "x": 0, "y": 160 },
    { "id": 6, "type": "IO", "mode": "Output", "label": "a", "x": -220, "y": 280 },
    { "id": 7, "type": "Process", "label": "temp = a + b", "x": -220, "y": 400 },
    { "id": 8, "type": "Process", "label": "a = b", "x": -220, "y": 520 },
    { "id": 9, "type": "Process", "label": "b = temp", "x": -220, "y": 640 },
    { "id": 10, "type": "Process", "label": "i = i + 1", "x": -220, "y": 760 },
    { "id": 11, "type": "StartStop", "mode": "Stop", "label": "Stop", "x": 200, "y": 280 }
  ],
  "connections": [
    { "from": 0,  "to": 1,  "isYes": false },
    { "from": 1,  "to": 2,  "isYes": false },
    { "from": 2,  "to": 3,  "isYes": false },
    { "from": 3,  "to": 4,  "isYes": false },
    { "from": 4,  "to": 5,  "isYes": false },
    { "from": 5,  "to": 6,  "isYes": true  },
    { "from": 5,  "to": 11, "isYes": false },
    { "from": 6,  "to": 7,  "isYes": false },
    { "from": 7,  "to": 8,  "isYes": false },
    { "from": 8,  "to": 9,  "isYes": false },
    { "from": 9,  "to": 10, "isYes": false },
    { "from": 10, "to": 5,  "isYes": false }
  ]
}
