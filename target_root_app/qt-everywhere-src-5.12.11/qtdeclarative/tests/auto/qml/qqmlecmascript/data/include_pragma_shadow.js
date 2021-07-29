.pragma library
var Shadowed = 2;
var global = (function(){return this})()

// set Shadowed on the global object as well. This should be different from
// the variable above, as the library has it's on context
global.Shadowed = 1;
