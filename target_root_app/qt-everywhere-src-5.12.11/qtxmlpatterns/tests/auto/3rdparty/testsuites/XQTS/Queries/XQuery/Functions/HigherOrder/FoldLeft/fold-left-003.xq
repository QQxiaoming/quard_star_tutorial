xquery version "1.1";
(: Higher Order Functions :)
(: fold-left function :)
(: Author - Michael Kay, Saxonica :)

<out>{fold-left(function($a, $b) { $a or $b }, false(), (true(), false(), false()))}</out>