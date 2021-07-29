(: Name: fn-name-6:)
(: Description: Evaluation of the fn:name function with the argument set to a non existing element.:)
(:Use of "fn:string-length" to avoid empty file.    :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works[1]/employee[2]) 
return fn:string-length(fn:name($h/child::text()[last()]))
