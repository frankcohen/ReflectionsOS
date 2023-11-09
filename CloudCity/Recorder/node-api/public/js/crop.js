var SIZE = 50
var stage = new createjs.Stage('canvas')

createjs.Touch.enable(stage)
stage.mouseMoveOutside = true

createjs.Ticker.on('tick', tick)
var coordinates = {
  xStart: 0,
  xEnd: 240,
  yStart: 0,
  yEnd: 240
}

var selection,
  g,
  r,
  moveListener,
  resizeHandle,
  resizing = false,
  maxX, maxY

function init_canvas() {
  selection = new createjs.Shape()
  g = selection.graphics
    .setStrokeStyle(3) 
    .beginStroke('#FEFACD')
    .beginFill('rgb(255,255,255,0.05)')
  r = g.drawRect(0, 0, 40, 40).command

   r.w = r.h
   selection.setBounds(coordinates.xStart, coordinates.yStart, r.w, r.h)

  resizeHandle = new createjs.Shape()
  resizeHandle.graphics.beginFill('#B6B6B6').drawRect(0, 0, 10, 10)

  const getLocalStorage = JSON.parse(localStorage.getItem('coordinates'))

  if (!getLocalStorage) {
    coordinates = {
      xStart: 0,
      xEnd: 240,
      yStart: 0,
      yEnd: 240
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
    // resizeHandle.x = selection.x + r.w - 5
    // resizeHandle.y = selection.y + r.h - 5
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
			
		  console.log(  );
          selection.x = ( evt.stageX - r.w ) / 2
          selection.y = ( evt.stageY - r.h ) / 2
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
	  
    coordinates.xEnd = Math.ceil( selection.x + r.w )
    coordinates.yEnd = Math.ceil( selection.y + r.h )
	
	if( ( coordinates.xEnd - coordinates.xStart ) < 240 ) 
	{ coordinates.xEnd = coordinates.xStart + 240 }
	if( ( coordinates.yEnd - coordinates.yStart ) < 240 ) 
	{ coordinates.yEnd = coordinates.yStart + 240 }

	coordinates.xStart = Math.ceil( coordinates.xStart )
	coordinates.yStart = Math.ceil( coordinates.yStart )
	coordinates.xEnd = Math.ceil( coordinates.xEnd )
	coordinates.yEnd = Math.ceil( coordinates.yEnd )
	  
    console.log(
      "Boundary moved/sized: ",
      "xStart ", coordinates.xStart,
      ", yStart ", coordinates.yStart,
	  ", xLength ", coordinates.xEnd - coordinates.xStart,
	  ", yLength ", coordinates.yEnd - coordinates.yStart
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
    r.h = size * 1.2
    resizeHandle.x = selection.x + r.w - 10
    resizeHandle.y = selection.y + r.h - 10
  }
}

function tick(event) {
  stage.update(event)
}
