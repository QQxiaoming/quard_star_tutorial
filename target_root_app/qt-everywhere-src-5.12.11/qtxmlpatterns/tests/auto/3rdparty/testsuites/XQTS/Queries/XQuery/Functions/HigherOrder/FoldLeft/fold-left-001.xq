xquery version "1.1";
(: Higher Order Functions :)
(: fold-left function :)
(: Author - Michael Kay, Saxonica :)

<out>{fold-left(function($a, $b) { $a + $b }, 0, 1 to 5)}</out>