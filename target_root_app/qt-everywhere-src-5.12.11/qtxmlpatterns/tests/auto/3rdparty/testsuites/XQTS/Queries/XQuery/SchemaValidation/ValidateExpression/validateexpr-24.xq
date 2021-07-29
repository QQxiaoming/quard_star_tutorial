declare namespace lu = 'http://www.w3.org/XQueryTest/ListUnionTypes' ;

(: name : validateexpr-24 :)
(: description : Test for the correct typed value when a list of union types is used. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTest/ListUnionTypes";
(: insert-end :)

let $v := validate { <lu:e>1 1.0e0</lu:e> }
let $dv := data($v)
return ($dv[1] instance of xs:integer, $dv[2] instance of xs:float)
