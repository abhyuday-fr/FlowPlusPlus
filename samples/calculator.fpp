{
  "version": 1,
  "nodes": [
    { "id": 0, "type": "StartStop", "mode": "Start", "label": "Start", "x": 0, "y": -320 },
    { "id": 1, "type": "IO", "mode": "Input", "label": "a", "x": 0, "y": -200 },
    { "id": 2, "type": "IO", "mode": "Input", "label": "b", "x": 0, "y": -80 },
    { "id": 3, "type": "Process", "label": "sum = a + b", "x": 0, "y": 40 },
    { "id": 4, "type": "Process", "label": "diff = a - b", "x": 0, "y": 160 },
    { "id": 5, "type": "Process", "label": "prod = a * b", "x": 0, "y": 280 },
    { "id": 6, "type": "IO", "mode": "Output", "label": "\"Sum: \" + sum", "x": 0, "y": 400 },
    { "id": 7, "type": "IO", "mode": "Output", "label": "\"Difference: \" + diff", "x": 0, "y": 520 },
    { "id": 8, "type": "IO", "mode": "Output", "label": "\"Product: \" + prod", "x": 0, "y": 640 },
    { "id": 9, "type": "StartStop", "mode": "Stop", "label": "Stop", "x": 0, "y": 760 }
  ],
  "connections": [
    { "from": 0, "to": 1, "isYes": false },
    { "from": 1, "to": 2, "isYes": false },
    { "from": 2, "to": 3, "isYes": false },
    { "from": 3, "to": 4, "isYes": false },
    { "from": 4, "to": 5, "isYes": false },
    { "from": 5, "to": 6, "isYes": false },
    { "from": 6, "to": 7, "isYes": false },
    { "from": 7, "to": 8, "isYes": false },
    { "from": 8, "to": 9, "isYes": false }
  ]
}
