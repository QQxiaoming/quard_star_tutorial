xquery version "1.1";
(: Higher Order Functions :)
(: filter function - item in sequence is of wrong type :)
(: Author - Michael Kay, Saxonica :)

<out>{filter(ends-with(?, 'e'), 
          ("apple", "pear", "apricot", "advocado", "orange", current-date()))}</out>