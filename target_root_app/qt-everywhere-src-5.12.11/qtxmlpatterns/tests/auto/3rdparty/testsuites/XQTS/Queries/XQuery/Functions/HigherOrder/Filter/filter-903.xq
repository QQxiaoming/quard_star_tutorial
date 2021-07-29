xquery version "1.1";
(: Higher Order Functions :)
(: filter function - returns non-singleton sequence :)
(: Author - Michael Kay, Saxonica :)

<out>{filter(function($x){if(starts-with($x,'a')) then (true(), true()) else false()}, 
          ("apple", "pear", "apricot", "advocado", "orange"))}</out>