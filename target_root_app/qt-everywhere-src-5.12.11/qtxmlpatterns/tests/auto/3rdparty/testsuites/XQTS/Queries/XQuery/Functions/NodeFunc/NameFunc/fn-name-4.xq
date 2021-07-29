(: Name: fn-name-4:)
(: Description: Evaluation of the fn:name function with the argument set to a document element.:)
(:Use of "fn:string-length" to avoid empty file.    :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:name($input-context1))
