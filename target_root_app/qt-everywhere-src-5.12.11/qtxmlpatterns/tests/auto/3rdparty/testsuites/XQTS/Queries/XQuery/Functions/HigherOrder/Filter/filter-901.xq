xquery version "1.1";
(: Higher Order Functions :)
(: filter function - not a boolean :)
(: Author - Michael Kay, Saxonica :)

<out>{filter(normalize-space#1, ("apple", "pear", "apricot", "advocado", "orange"))}</out>