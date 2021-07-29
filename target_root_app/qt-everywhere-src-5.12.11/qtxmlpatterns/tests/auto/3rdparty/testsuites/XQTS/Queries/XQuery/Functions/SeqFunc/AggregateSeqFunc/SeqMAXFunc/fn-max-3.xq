(: Name: fn-max-3:)
(: Description: Evaluation of type promotion when using mixed typed with fn:max function:)
(: Uses drived types promoted to least common type. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := fn:max((xs:long(20),xs:short(13)))
return $var instance of xs:integer