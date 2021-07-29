xquery version "1.1";
(: Higher Order Functions :)
(: inline function literal, constructor function, used-defined atomic type :)
(: Author - Michael Kay, Saxonica :)

import schema namespace a = "http://www.w3.org/XQueryTest/userDefinedTypes";


let $f := a:hatsize#1 return ($f(8) instance of a:hatsize)