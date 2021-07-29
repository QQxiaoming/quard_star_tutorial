(: Name: emptyorderdecl-9 :)
(: Description: Evaluation of a prolog that specifies "empty least" with a set of numbers (one empty) and no order by clause :)

declare default order empty least;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (1,4,0 div 0E0,7)
return $i