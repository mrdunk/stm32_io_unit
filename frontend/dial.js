'use strict';

window.onload = function () {
            // Creates canvas 320 × 200 at 100, 200
            var paper = Raphael(document.getElementById("paper"), 400, 400);


            var myTs = new TemperatureSensor(paper, tempSensorList, 80, 200, 50, 7, 9, 0, 40);
            var myTs2 = new TemperatureSensor(paper, tempSensorList, 250, 200, 80, 10, 12, 9, 31);

            window.setInterval(function(){
                myTs.updataGraph();
                myTs2.updataGraph();

            }, 50);
        
}


/* Return a colour appropriate for the angle requested. */
function colour(angle){
	return 'hsl(' + (0.25 - angle/360/4) + ', 0.99, 0.5)';
}


var tempSensorList = ['00000536d60c', '0000053610c1'];


function TemperatureSensor(paper, sensorList, centreX, centreY, baseRadius, lineThickness, lineSpacing, temperatureMin, temperatureMax){
	var dialSensitivity = 360 / (temperatureMax - temperatureMin)
	var dialOffset = temperatureMin * dialSensitivity;
	this.paper = paper;
	this.elementList = [];
	var diamiter = baseRadius;
	this.elementList.push(new TemperatureSensorElement('bar', 'bar', this.paper, centreX, centreY, diamiter, lineThickness, dialSensitivity, dialOffset));
	for(var sensor in sensorList){
		diamiter += lineSpacing;
		this.elementList.push(new TemperatureSensorElement(sensorList[sensor], 'arc', this.paper, centreX, centreY, diamiter, lineThickness, dialSensitivity, dialOffset));
	}

	this.populateData();
}
TemperatureSensor.prototype.populateData = function(){
	for(var key in this.elementList){
		var element = this.elementList[key];
		if(element.type === 'bar'){
			element.populateData(10);
		} else{
			if(element.identifier === '00000536d60c'){
				element.populateData(20);
			} else if(element.identifier === '0000053610c1'){
                                element.populateData(30);
                        }
		}
	}
};
TemperatureSensor.prototype.updataGraph = function(){
	var totalTemp = 0, totalCount = 0;
	var element, key;
	var barElement = this.elementList[0];

	for(key in this.elementList){
		element = this.elementList[key];
		if(key > 0){
			totalCount += 1;
			totalTemp += element.temperature;
		}
	}
	if(barElement.temperature < totalTemp / totalCount){
		barElement.baseGradient = '90-#526c7a-#64a0c1';
	} else {
		barElement.baseGradient = '90-#c13629-#ff5032';
	}

        for(key in this.elementList){
		element = this.elementList[key];
		element.updataGraph();
	}
};

/* A class for drawing 'rings' on the temperature sensor.
 * Args:
 *   type: 'arc' or 'bar'. 
 *         'arc' is a 'ring' around the outside of the dial for reperesenting
 *         temperatures.
 *         'bar' is a 'line' eminating from the centre of the dial intended as
 *         a controller to be dragged into position.
 */
function TemperatureSensorElement(identifier, type, paper, x, y, radius, thickness, dialSensitivity, dialOffset){
	this.identifier = identifier;
	this.type = type;
	this.paper = paper;
	this.x = x;
	this.y = y;
	this.radius = radius;
	this.thickness = thickness;
	this.dialSensitivity = dialSensitivity;
	this.dialOffset = dialOffset;
	this.inputX = 0;
	this.inputY = 0;

	console.log('registerd TemperatureSensorElement: ' + identifier);
}
TemperatureSensorElement.prototype.temperatureToAngle = function(temperature){
	return (temperature * this.dialSensitivity) - this.dialOffset;
};
TemperatureSensorElement.prototype.angleToTemperature = function(angle){
	return (angle + this.dialOffset) / this.dialSensitivity;
};
TemperatureSensorElement.prototype.populateData = function(setTemp){
	if (typeof setTemp !== 'undefined'){
		this.temperature = setTemp;
	} else if (typeof this.temperature === 'undefined'){
		this.temperature = 0;
	} else {
		this.temperature = this.temperature +1;
		if (this.temperature > 360){
			this.temperature = 0;
		}
	}
};
TemperatureSensorElement.prototype.setInput = function(dx, dy){
	this.inputX = dx;
	this.inputY = dy;
};
TemperatureSensorElement.prototype.setUp = function(){
                console.log('initialising pathObject');

                var arkShaddow = this.paper.circle(this.x, this.y, this.radius);
                arkShaddow.attr({'stroke-dasharray': '. ',});
                this.pathObject = this.paper.path();
                if(this.type === 'bar'){
			this.baseGradient = '90-#526c7a-#64a0c1';
                        this.baseCircle = this.paper.circle(this.x, this.y, this.radius);
                        this.baseCircle.attr({gradient: this.baseGradient});
                        this.baseCircle.node.onmouseover = function(){
                                this.style.cursor = 'pointer';
                        };
			// .bind will only on modern brousers... but that's ok here as so will our graphics library...
                        this.baseCircle.drag(this.setInput.bind(this), function(x, y){ /*drag start*/ }, function(){ /*ends*/ });
                        this.baseCircle.toBack();

			this.pathObject.node.onmouseover = function(){
                                this.style.cursor = 'pointer';
                        };
			this.pathObject.drag(this.setInput.bind(this), function(x, y){ /*drag start*/ }, function(){ /*ends*/ });
                }
};
TemperatureSensorElement.prototype.updataGraph = function(){
	var angleSet = this.temperatureToAngle(this.temperature);
	var _angle = angleSet * Math.PI / 180;

	if (typeof this.pathObject === 'undefined'){
		this.setUp();
	}

        if(this.type === 'bar'){
                if(this.inputX !== 0 || this.inputY !== 0){
                    var angleDeg = Math.atan2(this.inputX, -this.inputY) * 180 / Math.PI;
                    if(angleDeg < 0){ angleDeg = 360 + angleDeg;}
                    angleSet = (angleDeg + (3 * angleSet)) / 4;

                    this.temperature = this.angleToTemperature(angleSet);

		    this.baseCircle.attr({gradient: this.baseGradient});
                }
        }

	var path;
	if (this.type === 'arc'){
		if (angleSet !== 360){
			path = [['M', this.x, this.y - this.radius],
			     ['A', this.radius, this.radius, 0, +(angleSet > 180), 1,
			     this.x + this.radius * Math.sin(_angle), this.y - this.radius * Math.cos(_angle)]];
		} else {
			path = [['M',this.x, this.y - this.radius],
			     ['A', this.radius, this.radius, 0, 0, 1, this.x -0.1, this.y + this.radius]];
		}
	} else if (this.type === 'bar'){
		path = [['M', this.x, this.y], ['L', this.x + this.radius * Math.sin(_angle), this.y - this.radius * Math.cos(_angle)]];
	}
	this.pathObject.attr({'path': path,
			'stroke-linecap': 'round',
			'stroke': colour(angleSet),
			'stroke-width': this.thickness});
};

