(: Name: fn-name-16:)
(: Description: Evaluation of the fn:name function with argument that uses the "self" axes.  Returns a empty sequence:)
(: Uses fn:count to avoid empty file. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:count(fn:name($h/self::div))
