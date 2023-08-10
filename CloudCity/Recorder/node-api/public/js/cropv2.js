var SIZE = 50
var stage = new createjs.Stage('canvas')

createjs.Touch.enable(stage)
stage.mouseMoveOutside = true

createjs.Ticker.on('tick', tick)
var coordinates = {
  xStart: 0,
  xEnd: 0,
  yStart: 0,
  yEnd: 0
}

var selection,
  g,
  r,
  moveListener,
  resizeHandle,
  resizing = false

var maxX = 800
var maxY = 800

function init_canvas() {
  selection = new createjs.Shape()
  g = selection.graphics
    .setStrokeStyle(3)
    .beginStroke('#FEFACD')
    .beginFill('rgba(255,255,255,0.25)')
  r = g.drawRect(0, 0, 40, 40).command

  resizeHandle = new createjs.Shape()
  resizeHandle.graphics.beginFill('#B6B6B6').drawRect(0, 0, 10, 10)

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

  r.w = coordinates.xEnd - coordinates.xStart
  r.h = coordinates.yEnd - coordinates.yStart
  stage.addChild(selection).set({
    x: coordinates.xStart,
    y: coordinates.yStart
  })

  stage.addChild(resizeHandle).set({
    x: coordinates.xEnd - 10,
    y: coordinates.yEnd - 10
  })

  resizeHandle.on('mousedown', function (evt) {
    resizing = true
    stage.on('stagemousemove', drag)
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

    console.log(
      maxX,
      maxY,
      coordinates.xEnd - coordinates.xStart,
      coordinates.yEnd - coordinates.yStart
    )

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
    var width = Math.max(event.stageX - selection.x, 20)
    var height = Math.max(event.stageY - selection.y, 20)

    var size = Math.max(width, height) // always maintain a square shape

    r.w = size
    r.h = size
    resizeHandle.x = selection.x + r.w - 5
    resizeHandle.y = selection.y + r.h - 5
  }
}

function tick(event) {
  stage.update(event)
}
