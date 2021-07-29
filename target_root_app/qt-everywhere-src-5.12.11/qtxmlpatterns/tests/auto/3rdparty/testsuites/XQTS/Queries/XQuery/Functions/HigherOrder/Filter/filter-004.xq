xquery version "1.1";
(: Higher Order Functions :)
(: filter function :)
(: Author - Michael Kay, Saxonica :)



<out>{(1 to 20)[. = filter(function($x){$x idiv 2 * 2 = $x}, 1 to position())]}</out>