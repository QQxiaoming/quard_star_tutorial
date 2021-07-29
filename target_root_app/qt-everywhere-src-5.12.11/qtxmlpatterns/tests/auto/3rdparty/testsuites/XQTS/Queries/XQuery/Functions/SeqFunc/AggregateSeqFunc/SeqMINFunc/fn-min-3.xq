(: Name: fn-min-3:)
(: Description: Evaluation of type promotion when using mixed typed with fn:min function (used derived types):)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := fn:min((xs:long(22),xs:short(10)))
return $var instance of xs:integer