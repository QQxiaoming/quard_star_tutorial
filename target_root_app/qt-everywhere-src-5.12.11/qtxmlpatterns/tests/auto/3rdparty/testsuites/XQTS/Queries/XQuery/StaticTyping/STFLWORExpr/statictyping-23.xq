(: Name: statictyping-23 :)
(: Description: Evaluation of static typing where value of a variable is not set. :)
(: Used in a for clause of a FLOWR expression.                                    :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in (1, 2, 3)
for $z in ($x, $y)
return $x