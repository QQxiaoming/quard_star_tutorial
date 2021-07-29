xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function - exercise from XML Prague 2010 :)
(: Author - Michael Kay, Saxonica :)

<out>{
   let $in := 1 to 5
   return 
   map-pairs(function($a, $b){$a+$b}, $in, tail($in))
}</out>