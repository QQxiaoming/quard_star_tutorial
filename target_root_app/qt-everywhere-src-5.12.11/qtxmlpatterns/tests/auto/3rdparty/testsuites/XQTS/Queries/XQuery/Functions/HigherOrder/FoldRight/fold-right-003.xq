xquery version "1.1";
(: Higher Order Functions :)
(: fold-right function :)
(: Author - Michael Kay, Saxonica :)

<out>{fold-right(function($a, $b) { $a or $b }, false(), (true(), false(), false()))}</out>