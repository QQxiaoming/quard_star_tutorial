xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function :)
(: Author - Michael Kay, Saxonica :)

<out>{map-pairs(function($a as xs:integer, $b as xs:integer) as xs:integer{$a + $b}, 1 to 5, 1 to 5)}</out>