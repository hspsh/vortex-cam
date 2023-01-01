ring = 87;
bl = 70;
bw = 50;


sl=66/2;
sw=46/2;

$fn=100;

ring_depth = 4;
h= 12 + 8;

// TODO
// głębsze korytko na LED
// większe mejsce na płytkę, + miejsce na śrubki
// zwiększyć głebokość fest
// 45 stopni atapter
// kabel nie wschodzi

module case() {

difference() {
    color("grey") cylinder(h=h, d=ring+3);
    
    // cable hole
    translate([-10,0,0]) cube([100,17,(h-ring_depth-1)*2], center=true);
    
    intersection () {
        cylinder(h=h-ring_depth-1, d=ring);
        cube([bl*2,bw+15,(h-ring_depth-1)*2], center=true);
    }
    
    translate([0,0,h-ring_depth]) difference(){
        cylinder(h=ring_depth, d=ring);
        cylinder(h=ring_depth, d=ring-16);
    }
    
    translate([0,0,h-7]) cylinder(h=15, d1=10, d2=35);
    // LED pin hole
    led_d = 14;
    translate([ring/2-led_d/2 -1,0,h]) cube([led_d,10,30],center=true );
    // slide
    translate([11,ring/2-5]) rotate([0,-90,0]) linear_extrude(40) slide_side();
        translate([11,-(ring/2-5)]) rotate([0,-90,0]) linear_extrude(40) slide_side();
}


translate ([0,0,h-12]) union () {
    translate([sl, sw]) screw_sleeve();
    translate([-sl, sw]) screw_sleeve();
    translate([sl, -sw]) screw_sleeve();
    translate([-sl, -sw]) screw_sleeve();
}

}

module screw_sleeve(h=7) {
    difference() {
        cylinder(h, d=5);
        cylinder(h, d=2);
    }
}

module slide_side(outset=0.5) {
    minkowski() {
        projection() rotate ([0,90,0]) cylinder(2, 1.5, 2.5);
        circle(outset);
    }
}

//case();
mount();

module screw_hole() {
    cylinder(h=31, r=2, center=true);
    cylinder(h=20, r=5, center=true);
}

module mount() {
    d=ring+3;
    difference() {
        intersection () {
            cylinder(h=30, d=d, center=true);
            rotate([45,0,0]) cube([d,d/1.42,d/1.42], center=true);
            translate([-d/2,-d/2,-d]) cube([d,d,d]);
        }
        rotate([0,0,45]) linear_extrude(2) text("not7cd 2022, esp-vortex-cam", 4, halign="center", valign="center");
        
        translate([0,28,0]) rotate([45,0,0]) screw_hole();
        mirror([0,1,0]) translate([0,28,0]) rotate([45,0,0]) screw_hole();
        translate([-28,0,0]) screw_hole();
        translate([28,0,0]) screw_hole();

    }
    
    color("yellow") intersection () {
        cylinder(h=ring, d=ring+3, center=true);
        union() {
     translate([10,ring/2-5]) rotate([0,-90,0]) linear_extrude(40) slide_side(0.1);
     translate([10,-(ring/2-5)]) rotate([0,-90,0]) linear_extrude(40) slide_side(0.1);
        }
    }
}

