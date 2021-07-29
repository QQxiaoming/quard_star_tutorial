xquery version "1.1";
(: Higher Order Functions :)
(: filter function :)
(: Author - Michael Kay, Saxonica :)

<out>{filter(function($x){$x gt 10}, (12, 4, 46, 23, -8))}</out>