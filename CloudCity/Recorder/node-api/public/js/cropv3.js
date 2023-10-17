var SIZE = 50
var stage = new createjs.Stage('canvas')

createjs.Touch.enable(stage)
stage.mouseMoveOutside = true

createjs.Ticker.on('tick', tick)
var coordinates = {
  xStart: 40,
  xEnd: 280,
  yStart: 40,
  yEnd: 280
}

var selection,
  g,
  r,
  moveListener,
  resizeHandle,
  resizing = false

var maxX = 800
var maxY = 800

var selectionWidth, selectionHeight, selectionX, selectionY

function init_canvas() {
  selection = new createjs.Shape()
  g = selection.graphics
    .setStrokeStyle(3)
    .beginStroke('#FEFACD')
    .beginFill('rgba(255,255,255,0.1)')

  r = g.drawRect(
    0,
    0,
    coordinates.xEnd - coordinates.xStart,
    coordinates.yEnd - coordinates.yStart
  ).command

  resizeHandle = new createjs.Shape()
  resizeHandle.graphics.beginFill('#B6B6B6').drawRect(0, 0, 20, 20)

  r.w = coordinates.xEnd - coordinates.xStart
  r.h = coordinates.yEnd - coordinates.yStart

  selectionWidth = coordinates.xEnd - coordinates.xStart
  selectionHeight = coordinates.yEnd - coordinates.yStart
  selectionX = coordinates.xStart
  selectionY = coordinates.yStart

  const getLocalStorage = JSON.parse(localStorage.getItem('coordinates'))

  if (!getLocalStorage) {
    coordinates = {
      xStart: 0,
      xEnd: 40,
      yStart: 0,
      yEnd: 40
    }

    localStorage.setItem('coordinates', JSON.stringify(coordinates))
  } else {
    coordinates = getLocalStorage
  }

  stage.addChild(selection).set({
    x: coordinates.xStart,
    y: coordinates.yStart
  })

  stage.addChild(resizeHandle).set({
    x: coordinates.xEnd - 23,
    y: coordinates.yEnd - 23
  })

  resizeHandle.on('mousedown', function (evt) {
    resizing = true
    stage.on('stagemousemove', resize)
  })

  resizeHandle.on('pressup', function (evt) {
    resizing = false
    stage.off('stagemousemove', resize)

    // Saving to localStorage after resizing or dragging ends
    localStorage.setItem(
      'coordinates',
      JSON.stringify({
        xStart: coordinates.xStart.toString(),
        yStart: coordinates.yStart.toString(),
        xEnd: coordinates.xEnd.toString(),
        yEnd: coordinates.yEnd.toString()
      })
    )
  })

  stage.on('stagemousedown', function (evt) {
    if (
      !selection.hitTest(evt.stageX - selection.x, evt.stageY - selection.y)
    ) {
      if (!resizing) {
        return
      }
    } else {
      moveListener = stage.on('stagemousemove', function (evt) {
        if (!resizing) {
          selection.x = evt.stageX - r.w / 2
          selection.y = evt.stageY - r.h / 2
          resizeHandle.x = selection.x + r.w - 10
          resizeHandle.y = selection.y + r.h - 10

          coordinates.xStart = selection.x
          coordinates.yStart = selection.y
        }
      })
    }
  })

  stage.on('stagemouseup', function (evt) {
    stage.off('stagemousemove', moveListener)
    resizing = false

    coordinates.xEnd = selection.x + r.w
    coordinates.yEnd = selection.y + r.h

    // Saving to localStorage after resizing or dragging ends
    localStorage.setItem(
      'coordinates',
      JSON.stringify({
        xStart: coordinates.xStart.toString(),
        yStart: coordinates.yStart.toString(),
        xEnd: coordinates.xEnd.toString(),
        yEnd: coordinates.yEnd.toString()
      })
    )
  })

  stage.update()
}

function drag(event) {
  if (resizing) {
    var width = Math.max(event.stageX - selection.x, 23)
    var height = Math.max(event.stageY - selection.y, 23)

    var size = Math.max(width, height) // always maintain a square shape

    r.w = size
    r.h = size

    resizeHandle.x = selection.x
    resizeHandle.y = selection.y
  }
}

function tick(event) {
  stage.update(event)
}

function resize(event) {
  if (resizing) {
    var width = Math.max(event.stageX - selection.x, 23)
    var height = Math.max(event.stageY - selection.y, 23)

    var size = Math.max(width, height) // always maintain a square shape

    r.w = size
    r.h = size

    resizeHandle.x = selection.x + size - 20
    resizeHandle.y = selection.y + size - 20

    coordinates.xEnd = selection.x + size
    coordinates.yEnd = selection.y + size

    // Update the displayed rectangle dimensions
    r.command.w = size
    r.command.h = size
  }
}
