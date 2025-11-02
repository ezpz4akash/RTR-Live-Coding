function main(){
    const canvas = document.getElementById('glCanvas');

    if(canvas === null){
        console.log('Failed to retrieve the <canvas> element');
        return;
    }

    // Initialize the WebGL context
    const context = canvas.getContext('2d');

    if(context === null){
        console.log('Context cannot be obtained');
        return;
    }

    context.fillStyle = 'black';
    context.fillRect(0, 0, canvas.width, canvas.height);
    context.textalign = 'center';
    context.textBaseline = 'middle';
    context.font = '24px Arial';
    context.fillStyle = 'lime';

    var str = 'Hello World!!!';
    context.fillText(str, canvas.width / 2 - 60, canvas.height / 2);

    // Register event handlers
    window.addEventListener('keydown', keyDown, false);
    window.addEventListener('mousedown', mouseDown, false);
}

function keyDown(event){
    console.log('Key Down: ' + event.key);
}

function mouseDown(event){
    console.log('Mouse Button Pressed: ' + event.button);
}