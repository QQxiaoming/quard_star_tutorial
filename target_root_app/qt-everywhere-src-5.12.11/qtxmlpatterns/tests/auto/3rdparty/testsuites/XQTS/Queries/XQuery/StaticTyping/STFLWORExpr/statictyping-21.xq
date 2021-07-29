(: Name: statictyping-21 :)
(: Description: Evaluation of static typing where value of a variable is not set. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $x := 1
let $z := $x + $y
return $x