xquery version "1.1";
(: Higher Order Functions :)
(: filter function - returns empty sequence :)
(: Author - Michael Kay, Saxonica :)

<out>{filter(function($x){if(starts-with($x,'a')) then true() else ()}, 
          ("apple", "pear", "apricot", "advocado", "orange"))}</out>