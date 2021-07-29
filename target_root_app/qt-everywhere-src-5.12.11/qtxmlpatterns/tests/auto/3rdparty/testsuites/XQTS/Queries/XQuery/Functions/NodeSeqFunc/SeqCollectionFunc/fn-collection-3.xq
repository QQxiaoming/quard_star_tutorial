(: Name: fn-collection-3 :)
(: Description: Evaluation of ana fn:collection with argument set to an invalid URI.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:collection("invalidURI<>")