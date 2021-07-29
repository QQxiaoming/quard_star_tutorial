xquery version "1.1";
(: Higher Order Functions :)
(: fold-right function :)
(: Author - Michael Kay, Saxonica :)

<out>{fold-right(function($a, $b) { $a * $b }, 1, (2,3,5,7))}</out>