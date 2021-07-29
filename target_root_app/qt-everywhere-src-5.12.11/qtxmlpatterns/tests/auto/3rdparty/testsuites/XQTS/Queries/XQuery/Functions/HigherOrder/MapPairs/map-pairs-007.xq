xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function :)
(: Author - Michael Kay, Saxonica :)

<out>{
   let $millenium := year-from-date(current-date()) idiv 1000
   return 
   map-pairs(function($a, $b) as xs:integer* {1 to (string-length($a) + string-length($b))}, 
       ("a", "ab", "abc", ""), 
       ("", "", "", ""))}</out>