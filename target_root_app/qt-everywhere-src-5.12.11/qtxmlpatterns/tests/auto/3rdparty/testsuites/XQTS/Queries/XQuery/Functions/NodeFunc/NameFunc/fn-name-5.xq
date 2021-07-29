(: Name: fn-name-5:)
(: Description: Evaluation of the fn:name function with the argument set to a non existing element.:)
(:Use of "fn:string-length" to avoid empty file.    :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:name($input-context1/works[1]/nonexistent[1]))