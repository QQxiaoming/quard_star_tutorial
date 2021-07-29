xquery version "1.1";
(: Higher Order Functions :)
(: map function :)
(: Author - Michael Kay, Saxonica :)

<out>{map(function($x){$x[contains(., 'e')]}, ("john", "mary", "jane", "anne", "peter", "ian"))}</out>