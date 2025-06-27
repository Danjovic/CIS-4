diametroBase = 141;
alturaBase = 6;
encaixes = 6;
recesso = 2; 
folgaPCB = 3.5;
alturaCorpo = 25;
dimensaoMatriz = 4;  // 

dp    = 2.54; // decimo de polegada
borda = 3;
folga = 2;
passo = 9;  // 1 led a cada 9 furos

raioLED = (dp*passo/2)-folga/2;

difference(){
    union  () {
       base();    
       corpoDisplay();    
    }  
    union () {
        furosLeds();
        PCB();
    }
}



// furos dos leds
module furosLeds() {
desl = (dimensaoMatriz-1)/2*dp*passo; 
    translate ([-desl,-desl,0]) 
        for (i=[0:dimensaoMatriz-1])
            
    translate([0,i*dp*passo,0]){
        for (i=[0:dimensaoMatriz-1])
           translate([i*dp*passo,0,0])
             cylinder(alturaCorpo,r=raioLED);
    }
}

// base
module base() {
difference () {
    difference() {    
        cylinder(alturaBase,r=diametroBase/2);
        cylinder(alturaBase,r=diametroBase/2-11.5);  
    }
    for (i=[0:encaixes-1])// recessos
        rotate([0,0,360/encaixes*i])
            translate([diametroBase/2,0,alturaBase-recesso])
                cylinder(recesso,r=7);
    }
}


// corpo do display 
module corpoDisplay(){
hull (){
    desl = (dimensaoMatriz-1)/2*dp*passo; 
    translate([desl,desl,0])  cylinder(alturaCorpo,r=raioLED+borda);
    translate([desl,-desl,0]) cylinder(alturaCorpo,r=raioLED+borda);
    translate([-desl,desl,0]) cylinder(alturaCorpo,r=raioLED+borda);
    translate([-desl,-desl,0])cylinder(alturaCorpo,r=raioLED+borda);
    }
}

//PCB
module PCB() {
  translate([-38.73,-60,alturaCorpo-folgaPCB])
  cube([80,120,folgaPCB]);
}
