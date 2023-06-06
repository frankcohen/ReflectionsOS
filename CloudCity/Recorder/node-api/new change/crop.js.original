


var canvas;
var ctx;

var canvasOffset;
var offsetX;
var offsetY;

var isDrawing = false;

canvas = document.getElementById("canvas");
ctx = canvas.getContext("2d");

canvasOffset = $("#wrapper").offset();
offsetX = canvasOffset.left;
offsetY = canvasOffset.top;

$("#canvas").on('mousedown', function (e) {
    handleMouseDown(e);
}).on('mouseup', function(e) {
    handleMouseUp();
}).on('mousemove', function(e) {
    handleMouseMove(e);
});


var startX;
var startY;

function handleMouseUp() {
	isDrawing = false;
	canvas.style.cursor = "default";	
}

function handleMouseMove(e) {
	if (isDrawing) {
		var mouseX = parseInt(e.clientX - offsetX);
		var mouseY = parseInt(e.clientY - offsetY);				
		
		ctx.clearRect(0, 0, canvas.width, canvas.height);
		ctx.beginPath();
		ctx.rect(startX, startY, mouseX - startX, mouseY - startY);
		ctx.stroke();
		
	}
}

function handleMouseDown(e) {
	canvas.style.cursor = "crosshair";		
	isDrawing = true
	startX = parseInt(e.clientX - offsetX);
	startY = parseInt(e.clientY - offsetY);
}
function cropSelect() {
 

}


/*
var stage = new createjs.Stage("canvas");
createjs.Ticker.on("tick", tick);

var selection = new createjs.Shape(),
  g = selection.graphics.setStrokeStyle(1).beginStroke("#000").beginFill("rgba(0,0,0,0.05)"),
  sd = g.setStrokeDash([10, 5], 0).command,
  r = g.drawRect(0, 0, 100, 100).command,
  moveListener;


stage.on("stagemousedown", dragStart);
stage.on("stagemouseup", dragEnd);

function dragStart(event) {
  stage.addChild(selection).set({
    x: event.stageX,
    y: event.stageY
  });
  r.w = 0;
  r.h = 0;
  moveListener = stage.on("stagemousemove", drag);
};

function drag(event) {
  r.w = event.stageX - selection.x;
  r.h = event.stageY - selection.y;
}

function dragEnd(event) {
  stage.off("stagemousemove", moveListener);
}

function tick(event) {
  stage.update(event);
  sd.offset--;
}
*/
