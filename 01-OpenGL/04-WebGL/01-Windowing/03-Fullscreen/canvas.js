var canvas = null;
var context = null;
var bFullScreen = null;

function main(){
    canvas = document.getElementById('glCanvas');

    if(canvas === null){
        console.log('Failed to retrieve the <canvas> element');
        return;
    }

    // Initialize the WebGL context
    context = canvas.getContext('2d');

    if(context === null){
        console.log('Context cannot be obtained');
        return;
    }

    // Register event handlers
    window.addEventListener('keydown', keyDown, false);
    window.addEventListener('mousedown', mouseDown, false);

    drawText('Hello World!');
}

function keyDown(event){
    console.log('Key Down: ' + event.key);
    console.log('Key Down: ' + event.keyCode);

    switch(event.keyCode){
        case 70:  // 'F'
        case 102: // 'f'
            bFullScreen = !bFullScreen;
            toggleFullscreen();
        break;
    }
}

function mouseDown(event){
    console.log('Mouse Button Pressed: ' + event.button);
}

function toggleFullscreen(){
    var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
    if(fullscreenElement == null){
        if(canvas.requestFullscreen){
            canvas.requestFullscreen();
        } else if(canvas.mozRequestFullScreen){
            canvas.mozRequestFullScreen();
        } else if(canvas.webkitRequestFullscreen){
            canvas.webkitRequestFullscreen();
        } else if(canvas.msRequestFullscreen){
            canvas.msRequestFullscreen();
        }
    } else {
        if(document.exitFullscreen){
            document.exitFullscreen();
        } else if(document.mozCancelFullScreen){
            document.mozCancelFullScreen();
        } else if(document.webkitExitFullscreen){
            document.webkitExitFullscreen();
        } else if(document.msExitFullscreen){
            document.msExitFullscreen();
        }   
    }
}

function drawText(text){
    context.fillStyle = 'black';
    context.fillRect(0, 0, canvas.width, canvas.height);
    context.textalign = 'center';
    context.textBaseline = 'middle';
    context.font = '24px Arial';
    context.fillStyle = 'lime';
    context.fillText(text, canvas.width / 2 - 60, canvas.height / 2);
}