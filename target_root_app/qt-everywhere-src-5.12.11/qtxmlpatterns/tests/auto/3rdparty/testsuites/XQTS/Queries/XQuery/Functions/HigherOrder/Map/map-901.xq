xquery version "1.1";
(: Higher Order Functions :)
(: map function - error, function has the wrong arity :)
(: Author - Michael Kay, Saxonica :)

<out>{map(starts-with#2, ("aa", "bb", "cc", "dd", "ee"))}</out>