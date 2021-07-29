xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function :)
(: Author - Michael Kay, Saxonica :)

<out>{
   let $millenium := year-from-date(current-date()) idiv 1000
   return 
   map-pairs(function($a as xs:integer, $b as xs:integer) as xs:integer{$a + $b + $millenium}, 1 to 5, 2 to 6)}</out>