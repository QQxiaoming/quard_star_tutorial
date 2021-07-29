xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function, wrong arity function :)
(: Author - Michael Kay, Saxonica :)

<out>{map-pairs(deep-equal#3, ("aa", "bb", "cc", "dd", "ee"), ("AA", "BB", "cc", "dd", "EE"))}</out>