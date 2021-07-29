(: insert-start :)
import schema namespace ss="http://www.w3.org/query-test/complexSimple";
(: insert-end :)

let $v := validate { <ss:unionBase>1</ss:unionBase> }
let $dv := fn:data($v)
return fn:not($dv instance of xs:integer)
